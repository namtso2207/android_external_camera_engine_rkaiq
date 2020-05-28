/*
 *  Copyright (c) 2019 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <sys/stat.h>
#include "Isp20PollThread.h"
#include "Isp20StatsBuffer.h"
#include "rkisp2-config.h"
#include "SensorHw.h"
#include "Isp20_module_dbg.h"
#include <fcntl.h>

#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

#define CAPTURE_RAW_PATH "/tmp"
#define CAPTURE_CNT_FILENAME "capture_cnt"
// #define WRITE_ISP_REG
// #define WRITE_ISPP_REG
#define ISP_REGS_BASE 0xffb50000
#define ISPP_REGS_BASE 0xffb60000

#define RAW_FILE_IDENT 0x8080
#define HEADER_LEN 128U

/*
 * Raw file structure:
 *
+------------+-----------------+-------------+-----------------+---------------------------+
|    ITEM    |    PARAMETER    |  DATA TYPE  |  LENGTH(Bytes)  |        DESCRIPTION        |
+------------+-----------------+-------------+-----------------+---------------------------+
|            |     Identifier  |  uint16_t   |       2         |  fixed 0x8080             |
|            +-----------------+-------------+-----------------+---------------------------+
|            |  Header length  |  uint16_t   |       2         |  fixed 128U               |
|            +-----------------+-------------+-----------------+---------------------------+
|            |    Frame index  |  uint32_t   |       4         |                           |
|            +-----------------+-------------+-----------------+---------------------------+
|            |          Width  |  uint16_t   |       2         |  image width              |
|            +-----------------+-------------+-----------------+---------------------------+
|            |         Height  |  uint16_t   |       2         |  image height             |
|            +-----------------+-------------+-----------------+---------------------------+
|            |      Bit depth  |   uint8_t   |       1         |  image bit depth          |
|            +-----------------+-------------+-----------------+---------------------------+
|            |                 |             |                 |  0: BGGR;  1: GBRG;       |
|            |   Bayer format  |   uint8_t   |       1         |  2: GRBG;  3: RGGB;       |
|            +-----------------+-------------+-----------------+---------------------------+
|            |                 |             |                 |  1: linear                |
|    FRAME   |  Number of HDR  |             |                 |  2: long + short          |
|   HEADER   |      frame      |   uint8_t   |       1         |  3: long + mid + short    |
|            +-----------------+-------------+-----------------+---------------------------+
|            |                 |             |                 |  1: short                 |
|            |  Current frame  |             |                 |  2: mid                   |
|            |       type      |   uint8_t   |       1         |  3: long                  |
|            +-----------------+-------------+-----------------+---------------------------+
|            |   Storage type  |   uint8_t   |       1         |  0: packed; 1: unpacked   |
|            +-----------------+-------------+-----------------+---------------------------+
|            |    Line stride  |  uint16_t   |       2         |  In bytes                 |
|            +-----------------+-------------+-----------------+---------------------------+
|            |     Effective   |             |                 |                           |
|            |    line stride  |  uint16_t   |       2         |  In bytes                 |
|            +-----------------+-------------+-----------------+---------------------------+
|            |       Reserved  |   uint8_t   |      107        |                           |
+------------+-----------------+-------------+-----------------+---------------------------+
|            |                 |             |                 |                           |
|  RAW DATA  |       RAW DATA  |    RAW      |  W * H * bpp    |  RAW DATA                 |
|            |                 |             |                 |                           |
+------------+-----------------+-------------+-----------------+---------------------------+

 */

/*
 * the structure of measuure parameters from isp in meta_data file:
 *
 * "frame%08d-l_m_s-gain[%08.5f_%08.5f_%08.5f]-time[%08.5f_%08.5f_%08.5f]-awbGain[%08.4f_%08.4f_%08.4f_%08.4f]-dgain[%08d]"
 *
 */

namespace RkCam {

const struct capture_fmt Isp20PollThread::csirx_fmts[] =
{
    /* raw */
    {
	.fourcc = V4L2_PIX_FMT_SRGGB8,
	.bayer_fmt = 3,
	.pcpp = 1,
	.bpp = { 8 },
    }, {
	.fourcc = V4L2_PIX_FMT_SGRBG8,
	.bayer_fmt = 2,
	.pcpp = 1,
	.bpp = { 8 },
    }, {
	.fourcc = V4L2_PIX_FMT_SGBRG8,
	.bayer_fmt = 1,
	.pcpp = 1,
	.bpp = { 8 },
    }, {
	.fourcc = V4L2_PIX_FMT_SBGGR8,
	.bayer_fmt = 0,
	.pcpp = 1,
	.bpp = { 8 },
    }, {
	.fourcc = V4L2_PIX_FMT_SRGGB10,
	.bayer_fmt = 3,
	.pcpp = 4,
	.bpp = { 10 },
    }, {
	.fourcc = V4L2_PIX_FMT_SGRBG10,
	.bayer_fmt = 2,
	.pcpp = 4,
	.bpp = { 10 },
    }, {
	.fourcc = V4L2_PIX_FMT_SGBRG10,
	.bayer_fmt = 1,
	.pcpp = 4,
	.bpp = { 10 },
    }, {
	.fourcc = V4L2_PIX_FMT_SBGGR10,
	.bayer_fmt = 0,
	.pcpp = 4,
	.bpp = { 10 },
    }, {
	.fourcc = V4L2_PIX_FMT_SRGGB12,
	.pcpp = 2,
	.bpp = { 12 },
    }, {
	.fourcc = V4L2_PIX_FMT_SGRBG12,
	.pcpp = 2,
	.bpp = { 12 },
    }, {
	.fourcc = V4L2_PIX_FMT_SGBRG12,
	.bayer_fmt = 1,
	.pcpp = 2,
	.bpp = { 12 },
    }, {
	.fourcc = V4L2_PIX_FMT_SBGGR12,
	.bayer_fmt = 0,
	.pcpp = 2,
	.bpp = { 12 },
    },
};

const char*
Isp20PollThread::mipi_poll_type_to_str[ISP_POLL_MIPI_MAX] =
{
    "mipi_tx_poll",
    "mipi_rx_poll",
};

class MipiPollThread
    : public Thread
{
public:
    MipiPollThread (Isp20PollThread*poll, int type, int dev_index)
        : Thread (Isp20PollThread::mipi_poll_type_to_str[type])
        , _poll (poll)
        , _type (type)
        , _dev_index (dev_index)
    {}

protected:
    virtual bool loop () {
        XCamReturn ret = _poll->mipi_poll_buffer_loop (_type, _dev_index);

        if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_ERROR_TIMEOUT ||
                XCAM_RETURN_BYPASS)
            return true;
        return false;
    }

private:
    Isp20PollThread *_poll;
    int _type;
    int _dev_index;
};

bool
Isp20PollThread::get_value_from_file(const char* path, int* value)
{
    char buffer[8] = {0};
    int fp;

    fp = open(path, O_RDONLY | O_SYNC);
    if (fp) {
	if (read(fp, buffer, sizeof(buffer)) <= 0)
	    LOGW_CAMHW("%s read %s failed!\n", __func__, path);
	else
	    *value = atoi(buffer);
	close(fp);
	return true;
    }

    return false;
}


bool
Isp20PollThread::set_value_to_file(const char* path, int value)
{
    char buffer[8] = {0};
    int fp;

    fp = open(path, O_CREAT | O_RDWR | O_SYNC);
    if (fp) {
	ftruncate(fp,0);
	lseek(fp, 0, SEEK_SET);
	snprintf(buffer, sizeof(buffer), "%d", _capture_raw_num);
	if (write(fp, buffer, sizeof(buffer)) <= 0)
		LOGW_CAMHW("%s write %s failed!\n", __func__, path);
	close(fp);
	return true;
    }

    return false;
}

SmartPtr<VideoBuffer>
Isp20PollThread::new_video_buffer(SmartPtr<V4l2Buffer> buf,
                                  SmartPtr<V4l2Device> dev,
                                  int type)
{
    ENTER_CAMHW_FUNCTION();
    SmartPtr<VideoBuffer> video_buf = nullptr;

    if (type == ISP_POLL_3A_STATS) {
        SmartPtr<RkAiqIspParamsProxy> ispParams = nullptr;
        SmartPtr<RkAiqExpParamsProxy> expParams = nullptr;

        _event_handle_dev->getEffectiveExpParams(expParams, buf->get_buf().sequence);
	if (_rx_handle_dev)
		_rx_handle_dev->getEffectiveIspParams(ispParams, buf->get_buf().sequence);

	if (_capture_metadata_num > 0) {
	    if (_is_raw_dir_exist && ispParams.ptr() && expParams.ptr())
		write_metadata_to_file(raw_dir_path,
				       buf->get_buf().sequence,
				       ispParams, expParams);
	    _capture_metadata_num--;
	    if (!_capture_metadata_num) {
		_is_raw_dir_exist = false;
		LOGD_CAMHW("stop capturing raw!\n");
	    }
	}

        // TODO: get ispParams from isp dev;

        video_buf = new Isp20StatsBuffer(buf, dev, ispParams, expParams);
    } else
        return PollThread::new_video_buffer(buf, dev, type);
    EXIT_CAMHW_FUNCTION();

    return video_buf;
}

XCamReturn
Isp20PollThread::notify_sof (int64_t time, int frameid)
{
    ENTER_CAMHW_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    XCAM_ASSERT(_event_handle_dev.ptr());
    ret = _event_handle_dev->handle_sof(time, frameid);
    if (get_rkaiq_runtime_dbg() > 0) {
        XCAM_STATIC_FPS_CALCULATION(SOF_FPS, 60);
    }
    EXIT_CAMHW_FUNCTION();

    return ret;
}

bool
Isp20PollThread::set_event_handle_dev(SmartPtr<SensorHw> &dev)
{
    ENTER_CAMHW_FUNCTION();
    XCAM_ASSERT (dev.ptr());
    _event_handle_dev = dev;
    EXIT_CAMHW_FUNCTION();
    return true;
}

bool
Isp20PollThread::set_rx_handle_dev(CamHwIsp20* dev)
{
    ENTER_CAMHW_FUNCTION();
    XCAM_ASSERT (dev);
    _rx_handle_dev = dev;
    EXIT_CAMHW_FUNCTION();
    return true;
}

Isp20PollThread::Isp20PollThread()
    : PollThread()
    , _first_trigger(true)
    , sns_width(0)
    , sns_height(0)
    , _is_raw_dir_exist(false)
    , _is_capture_raw(false)
    , _capture_raw_num(0)
    , _capture_metadata_num(0)

{
    for (int i = 0; i < 3; i++) {
        SmartPtr<MipiPollThread> mipi_poll = new MipiPollThread(this, ISP_POLL_MIPI_TX, i);
        XCAM_ASSERT (mipi_poll.ptr ());
        _isp_mipi_tx_infos[i].loop = mipi_poll;

        mipi_poll = new MipiPollThread(this, ISP_POLL_MIPI_RX, i);
        XCAM_ASSERT (mipi_poll.ptr ());
        _isp_mipi_rx_infos[i].loop = mipi_poll;

        _isp_mipi_tx_infos[i].stop_fds[0] = -1;
        _isp_mipi_tx_infos[i].stop_fds[1] = -1;
        _isp_mipi_rx_infos[i].stop_fds[0] = -1;
        _isp_mipi_rx_infos[i].stop_fds[1] = -1;

    }

    LOGD_CAMHW("Isp20PollThread constructed");
}

Isp20PollThread::~Isp20PollThread()
{
    stop();

    LOGD_CAMHW("~Isp20PollThread destructed");
}


XCamReturn
Isp20PollThread::start ()
{
    if (create_stop_fds_mipi()) {
        LOGE_CAMHW("create mipi stop fds failed !");
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    for (int i = 0; i < _mipi_dev_max; i++) {
        _isp_mipi_tx_infos[i].loop->start ();
        _isp_mipi_rx_infos[i].loop->start ();
    }
    return PollThread::start ();
}

XCamReturn
Isp20PollThread::stop ()
{
    for (int i = 0; i < _mipi_dev_max; i++) {
        if (_isp_mipi_tx_infos[i].dev.ptr ()) {
            if (_isp_mipi_tx_infos[i].stop_fds[1] != -1) {
                char buf = 0xf;  // random value to write to flush fd.
                unsigned int size = write(_isp_mipi_tx_infos[i].stop_fds[1], &buf, sizeof(char));
                if (size != sizeof(char))
                    LOGW_CAMHW("Flush write not completed");
            }
            _isp_mipi_tx_infos[i].loop->stop ();
            _isp_mipi_tx_infos[i].buf_list.clear ();
            _isp_mipi_tx_infos[i].cache_list.clear ();
        }

        if (_isp_mipi_rx_infos[i].dev.ptr ()) {
            if (_isp_mipi_rx_infos[i].stop_fds[1] != -1) {
                char buf = 0xf;  // random value to write to flush fd.
                unsigned int size = write(_isp_mipi_rx_infos[i].stop_fds[1], &buf, sizeof(char));
                if (size != sizeof(char))
                    LOGW_CAMHW("Flush write not completed");
            }
            _isp_mipi_rx_infos[i].loop->stop ();
            _isp_mipi_rx_infos[i].buf_list.clear ();
            _isp_mipi_rx_infos[i].cache_list.clear ();
        }
    }

    _isp_hdr_fid2times_map.clear();
    _isp_hdr_fid2ready_map.clear();
    destroy_stop_fds_mipi ();

    return PollThread::stop ();
}

void
Isp20PollThread::set_working_mode(int mode)
{
   _working_mode = mode;

    switch (_working_mode) {
        case RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR:
        case RK_AIQ_ISP_HDR_MODE_3_LINE_HDR:
            _mipi_dev_max = 3;
            break;
        case RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR:
        case RK_AIQ_ISP_HDR_MODE_2_LINE_HDR:
            _mipi_dev_max = 2;
            break;
        default:
            _mipi_dev_max = 1;
    }
}

void
Isp20PollThread::set_mipi_devs(SmartPtr<V4l2Device> mipi_tx_devs[3],
                               SmartPtr<V4l2Device> mipi_rx_devs[3],
                               SmartPtr<V4l2SubDevice> isp_dev)
{
    _isp_core_dev = isp_dev;
    for (int i = 0; i < 3; i++) {
        _isp_mipi_tx_infos[i].dev = mipi_tx_devs[i];
        _isp_mipi_rx_infos[i].dev = mipi_rx_devs[i];
    }
}

XCamReturn
Isp20PollThread::hdr_mipi_start(SmartPtr<SensorHw> sensor)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!sensor.ptr()) {
        //TODO no sensor case
        return XCAM_RETURN_ERROR_SENSOR;
    } else {
        rk_aiq_exposure_sensor_descriptor sns_des;
        sensor->get_format(&sns_des);
	sns_width = sns_des.sensor_output_width;
	sns_height = sns_des.sensor_output_height;
        pixelformat = sns_des.sensor_pixelformat;
    }

    // mipi rx/tx format should match to sensor.
    for (int i = 0; i < _mipi_dev_max; i++) {
        _isp_mipi_tx_infos[i].dev->set_format(sns_width, sns_height, pixelformat,
                                              V4L2_FIELD_NONE, 0);
        ret = _isp_mipi_tx_infos[i].dev->start();
        if (ret < 0)
            LOGE_CAMHW("mipi tx:%d start err: %d\n", ret);

        _isp_mipi_rx_infos[i].dev->set_format(sns_width, sns_height, pixelformat,
                                              V4L2_FIELD_NONE, 0);
        ret = _isp_mipi_rx_infos[i].dev->start();
        if (ret < 0)
            LOGE_CAMHW("mipi rx:%d start err: %d\n", ret);
    }

    return ret;
}

XCamReturn
Isp20PollThread::hdr_mipi_stop()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    for (int i = 0; i < _mipi_dev_max; i++) {
        ret = _isp_mipi_tx_infos[i].dev->stop();
        if (ret < 0)
            LOGE_CAMHW("mipi tx:%d stop err: %d\n", ret);
        ret = _isp_mipi_rx_infos[i].dev->stop();
        if (ret < 0)
            LOGE_CAMHW("mipi rx:%d stop err: %d\n", ret);
    }

    return ret;
}

void
Isp20PollThread::set_hdr_frame_readback_infos(int frame_id, int times)
{
    _mipi_trigger_mutex.lock();
    _isp_hdr_fid2times_map[frame_id] = times;
    LOGD_CAMHW("rdtimes seq %d \n", frame_id);
    _mipi_trigger_mutex.unlock();
    trigger_readback();
}

XCamReturn
Isp20PollThread::create_stop_fds_mipi () {
    int i, status = 0;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    destroy_stop_fds_mipi();

    for (i = 0; i < _mipi_dev_max; i++) {
        status = pipe(_isp_mipi_tx_infos[i].stop_fds);
        if (status < 0) {
            LOGE_CAMHW("Failed to create mipi tx:%d poll stop pipe: %s",
                            i, strerror(errno));
            ret = XCAM_RETURN_ERROR_UNKNOWN;
            goto exit_error;
        }
        status = fcntl(_isp_mipi_tx_infos[0].stop_fds[0], F_SETFL, O_NONBLOCK);
        if (status < 0) {
            LOGE_CAMHW("Fail to set event mipi tx:%d stop pipe flag: %s",
                            i, strerror(errno));
            goto exit_error;
        }

        status = pipe(_isp_mipi_rx_infos[i].stop_fds);
        if (status < 0) {
            LOGE_CAMHW("Failed to create mipi rx:%d poll stop pipe: %s",
                            i, strerror(errno));
            ret = XCAM_RETURN_ERROR_UNKNOWN;
            goto exit_error;
        }
        status = fcntl(_isp_mipi_rx_infos[0].stop_fds[0], F_SETFL, O_NONBLOCK);
        if (status < 0) {
            LOGE_CAMHW("Fail to set event mipi rx:%d stop pipe flag: %s",
                            i, strerror(errno));
            goto exit_error;
        }
    }

    return XCAM_RETURN_NO_ERROR;
exit_error:
    destroy_stop_fds_mipi();
    return ret;
}

void Isp20PollThread::destroy_stop_fds_mipi () {
    for (int i = 0; i < 3; i++) {
        if (_isp_mipi_tx_infos[i].stop_fds[0] != -1 ||
            _isp_mipi_tx_infos[i].stop_fds[1] != -1) {
            close(_isp_mipi_tx_infos[i].stop_fds[0]);
            close(_isp_mipi_tx_infos[i].stop_fds[1]);
            _isp_mipi_tx_infos[i].stop_fds[0] = -1;
            _isp_mipi_tx_infos[i].stop_fds[1] = -1;
        }

        if (_isp_mipi_rx_infos[i].stop_fds[0] != -1 ||
            _isp_mipi_rx_infos[i].stop_fds[1] != -1) {
            close(_isp_mipi_rx_infos[i].stop_fds[0]);
            close(_isp_mipi_rx_infos[i].stop_fds[1]);
            _isp_mipi_rx_infos[i].stop_fds[0] = -1;
            _isp_mipi_rx_infos[i].stop_fds[1] = -1;
        }
    }
}

XCamReturn
Isp20PollThread::mipi_poll_buffer_loop (int type, int dev_index)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int poll_ret = 0;
    SmartPtr<V4l2Buffer> buf;
    SmartPtr<V4l2Device> dev;
    int stop_fd = -1;

    if (type == ISP_POLL_MIPI_TX) {
        dev = _isp_mipi_tx_infos[dev_index].dev;
        stop_fd = _isp_mipi_tx_infos[dev_index].stop_fds[0];
    } else if (type == ISP_POLL_MIPI_RX) {
        dev = _isp_mipi_rx_infos[dev_index].dev;
        stop_fd = _isp_mipi_rx_infos[dev_index].stop_fds[0];
    } else
        return XCAM_RETURN_ERROR_UNKNOWN;

    poll_ret = dev->poll_event (PollThread::default_poll_timeout,
                                stop_fd);

    if (poll_ret == POLL_STOP_RET) {
        LOGW_CAMHW("poll %s buffer stop success !", mipi_poll_type_to_str[type]);
        // stop success, return error to stop the poll thread
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    if (poll_ret <= 0) {
        LOGW_CAMHW("mipi_dev_index %d poll %s buffer event got error(0x%x) but continue\n",
                        dev_index, mipi_poll_type_to_str[type], poll_ret);
        ::usleep (10000); // 10ms
        return XCAM_RETURN_ERROR_TIMEOUT;
    }

    ret = dev->dequeue_buffer (buf);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGW_CAMHW("dequeue %s buffer failed", mipi_poll_type_to_str[type]);
        return ret;
    }

    SmartPtr<V4l2BufferProxy> buf_proxy = new V4l2BufferProxy(buf, dev);
    if (type == ISP_POLL_MIPI_TX) {
        handle_tx_buf(buf_proxy, dev_index);
    } else if (type == ISP_POLL_MIPI_RX) {
        handle_rx_buf(buf_proxy, dev_index);
    }

    return ret;
}

void
Isp20PollThread::handle_rx_buf(SmartPtr<V4l2BufferProxy> &rx_buf, int dev_index)
{

    SmartPtr<V4l2BufferProxy> buf = _isp_mipi_rx_infos[dev_index].buf_list.pop(-1);
    LOGD_CAMHW("%s dev_index:%d index:%d fd:%d\n",
        __func__, dev_index, buf->get_v4l2_buf_index(), buf->get_expbuf_fd());
}

void Isp20PollThread::sync_tx_buf()
{
    SmartPtr<V4l2BufferProxy> buf_s, buf_m, buf_l;
    uint32_t sequence_s = -1, sequence_m = -1, sequence_l = -1;

    _mipi_buf_mutex.lock();
    for (int i = 0; i < _mipi_dev_max; i++) {
        if (_isp_mipi_tx_infos[i].buf_list.is_empty()) {
            _mipi_buf_mutex.unlock();
            return;
        }
    }

    buf_l = _isp_mipi_tx_infos[ISP_MIPI_HDR_L].buf_list.front();
    if (buf_l.ptr())
        sequence_l = buf_l->get_sequence();

    buf_m = _isp_mipi_tx_infos[ISP_MIPI_HDR_M].buf_list.front();
    if (buf_m.ptr())
        sequence_m = buf_m->get_sequence();

    buf_s = _isp_mipi_tx_infos[ISP_MIPI_HDR_S].buf_list.front();

    if (buf_s.ptr()) {
        sequence_s = buf_s->get_sequence();
        if ((_working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
             _working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) &&
            buf_m.ptr() && buf_l.ptr() && buf_s.ptr() &&
            sequence_l == sequence_s && sequence_m == sequence_s) {
            _isp_mipi_rx_infos[ISP_MIPI_HDR_S].cache_list.push(buf_s);
            _isp_mipi_rx_infos[ISP_MIPI_HDR_M].cache_list.push(buf_m);
            _isp_mipi_rx_infos[ISP_MIPI_HDR_L].cache_list.push(buf_l);

            _isp_mipi_tx_infos[ISP_MIPI_HDR_S].buf_list.erase(buf_s);
            _isp_mipi_tx_infos[ISP_MIPI_HDR_M].buf_list.erase(buf_m);
            _isp_mipi_tx_infos[ISP_MIPI_HDR_L].buf_list.erase(buf_l);
            _mipi_trigger_mutex.lock();
            _isp_hdr_fid2ready_map[sequence_s] = true;
            _mipi_trigger_mutex.unlock();
            LOGD_CAMHW("trigger readback %d", sequence_s);
            trigger_readback();
        } else if ((_working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
                    _working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) &&
                   buf_m.ptr() && buf_s.ptr() && sequence_m == sequence_s) {
            _isp_mipi_rx_infos[ISP_MIPI_HDR_S].cache_list.push(buf_s);
            _isp_mipi_rx_infos[ISP_MIPI_HDR_M].cache_list.push(buf_m);

            _isp_mipi_tx_infos[ISP_MIPI_HDR_S].buf_list.erase(buf_s);
            _isp_mipi_tx_infos[ISP_MIPI_HDR_M].buf_list.erase(buf_m);
            _mipi_trigger_mutex.lock();
            _isp_hdr_fid2ready_map[sequence_s] = true;
            _mipi_trigger_mutex.unlock();
            LOGD_CAMHW("trigger readback %d", sequence_s);
            trigger_readback();
        } else if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            _isp_mipi_rx_infos[ISP_MIPI_HDR_S].cache_list.push(buf_s);
            _isp_mipi_tx_infos[ISP_MIPI_HDR_S].buf_list.erase(buf_s);
            _mipi_trigger_mutex.lock();
            _isp_hdr_fid2ready_map[sequence_s] = true;
            _mipi_trigger_mutex.unlock();
            LOGD_CAMHW("trigger readback %d", sequence_s);
            trigger_readback();
        } else {
            LOGW_CAMHW("do nothing, sequence not match l: %d, s: %d, m: %d !!!",
                           sequence_l, sequence_s, sequence_m);
        }
    }
    _mipi_buf_mutex.unlock();
}

void
Isp20PollThread::handle_tx_buf(SmartPtr<V4l2BufferProxy> &tx_buf, int dev_index)
{
    LOGD_CAMHW("%s dev_index:%d sequence:%d\n",
        __func__, dev_index, tx_buf->get_sequence());
    _isp_mipi_tx_infos[dev_index].buf_list.push(tx_buf);
    sync_tx_buf();
}

void
Isp20PollThread::trigger_readback()
{
    std::map<uint32_t, bool>::iterator it_ready;
    SmartPtr<V4l2Buffer> v4l2buf;
    SmartPtr<V4l2BufferProxy> buf_proxy;
    uint32_t sequence = -1;
    int times = -1;

    _mipi_trigger_mutex.lock();
    _isp_hdr_fid2ready_map.size();
    if (_isp_hdr_fid2ready_map.size() == 0) {
        LOGD_CAMHW("%s buf not ready !", __func__);
        _mipi_trigger_mutex.unlock();
        return;
    }

    it_ready = _isp_hdr_fid2ready_map.begin();
    sequence = it_ready->first;

    std::map<uint32_t, int>::iterator it_times_del;
    for (std::map<uint32_t, int>::iterator iter = _isp_hdr_fid2times_map.begin();
         iter != _isp_hdr_fid2times_map.end();) {
        if (iter->first < sequence) {
            it_times_del = iter++;
            LOGD_CAMHW("del seq %d", it_times_del->first);
            _isp_hdr_fid2times_map.erase(it_times_del);
        } else if (iter->first == sequence) {
            times = iter->second;
            it_times_del = iter++;
            LOGD_CAMHW("del seq %d", it_times_del->first);
            _isp_hdr_fid2times_map.erase(it_times_del);
            break;
        } else {
            LOGW_CAMHW("%s missing rdtimes for buf_seq %d, min rdtimes_seq %d !",
                           __func__, sequence, iter->first);
            times = 0;
            break;
        }
    }

    if (times == -1) {
        LOGD_CAMHW("%s rdtimes not ready for seq %d !", __func__, sequence);
        _mipi_trigger_mutex.unlock();
        return;
    }

    _isp_hdr_fid2ready_map.erase(it_ready);

    struct isp2x_csi_trigger tg = {
        .frame_id = sequence,
        .times = times - 1,
    };

    if (_first_trigger) {
        tg.times = 1;
        _first_trigger = false;
    }

    /* TODO: fix the trigger time for ahdr temporarily */
    if (_mipi_dev_max == 1)
        tg.times = 1;
    else
        tg.times = 2;
    LOGW_CAMHW("%s fix the trigger to %d time for ahdr temporarily\n",
	       __func__,
	       tg.times);

    LOGD_CAMHW("%s set frame[%d] isp params, readback %d times, capturing on frame%d\n",
	       __func__, sequence, tg.times,
	       _capture_raw_num);

    if (_rx_handle_dev) {
        if (_rx_handle_dev->setIspParamsSync(sequence) ||
            _rx_handle_dev->setIsppParamsSync(sequence)) {
            LOGE_CAMHW("%s frame[%d] set isp params failed, don't read back!\n",
                       __func__, sequence);
            // drop frame, return buf to tx
            for (int i = 0; i < _mipi_dev_max; i++) {
                _isp_mipi_rx_infos[i].cache_list.pop(-1);
            }
        } else {
            char file_name[32] = {0};
	    int ret = XCAM_RETURN_NO_ERROR;

	    snprintf(file_name, sizeof(file_name), "%s/%s",
		     CAPTURE_RAW_PATH, CAPTURE_CNT_FILENAME);
	    if (!_is_capture_raw) {
		get_value_from_file(file_name, &_capture_raw_num);

		if (_capture_raw_num > 0) {
		    _is_capture_raw = true;
		    _capture_metadata_num = _capture_raw_num;
		}
	    }

            for (int i = 0; i < _mipi_dev_max; i++) {
		ret = _isp_mipi_rx_infos[i].dev->get_buffer(v4l2buf);
		if (ret != XCAM_RETURN_NO_ERROR) {
			LOGE_CAMHW("Rx[%d] can not get buffer\n", i);
			break;
		} else {
		    buf_proxy = _isp_mipi_rx_infos[i].cache_list.pop(-1);
		    _isp_mipi_rx_infos[i].buf_list.push(buf_proxy);

		    v4l2buf->set_expbuf_fd(buf_proxy->get_expbuf_fd());
		    ret = _isp_mipi_rx_infos[i].dev->queue_buffer(v4l2buf);
		    if (ret != XCAM_RETURN_NO_ERROR) {
			_isp_mipi_rx_infos[i].buf_list.pop(-1);
			LOGE_CAMHW("Rx[%d] queue buffer failed\n", i);
			break;
		    }
		}

                if (_is_capture_raw && _capture_raw_num > 0) {
                    if (!_is_raw_dir_exist)
                        creat_raw_dir(CAPTURE_RAW_PATH);

                    if (_is_raw_dir_exist) {
			char raw_name[128] = {0};
			FILE *fp = nullptr;

                        XCAM_STATIC_PROFILING_START(write_raw);
#ifdef WRITE_ISP_REG
			write_reg_to_file(ISP_REGS_BASE, 0x0, 0x6000, sequence);
#endif

			memset(raw_name, 0, sizeof(raw_name));
			snprintf(raw_name, sizeof(raw_name),
				 "%s/frame%d_%dx%d_%s.raw",
				 raw_dir_path,
				 sequence,
				 sns_width,
				 sns_height,
				 i == 0 ? "short" : "long");

			fp = fopen(raw_name, "wb+");
			if (fp != nullptr) {
			    write_frame_header_to_raw(fp, i, sequence);
			    write_raw_to_file(fp, i, sequence,
					      (void *)(buf_proxy->get_v4l2_userptr()),
					      v4l2buf->get_buf().m.planes[0].bytesused);
			    fclose(fp);
			}
                        XCAM_STATIC_PROFILING_END(write_raw, 0);
                    }
                }
            }

	    if (_is_capture_raw) {
		_capture_raw_num--;
		if (!_capture_raw_num) {
		    set_value_to_file(file_name, _capture_raw_num);
		    _is_capture_raw = false;
		}
	    }

	    if (ret == XCAM_RETURN_NO_ERROR)
                _isp_core_dev->io_control(RKISP_CMD_TRIGGER_READ_BACK, &tg);
	    else
		LOGE_CAMHW("%s frame[%d] queue  failed, don't read back!\n",
			   __func__, sequence);
        }
    }

    _mipi_trigger_mutex.unlock();
}

void
Isp20PollThread::write_metadata_to_file(const char* dir_path,
		int frame_id,
		SmartPtr<RkAiqIspParamsProxy>& ispParams,
		SmartPtr<RkAiqExpParamsProxy>& expParams)
{
    FILE *fp = nullptr;
    char file_name[64] = {0};
    char buffer[256] = {0};

    snprintf(file_name, sizeof(file_name), "%s/meta_data", dir_path);

    fp = fopen(file_name, "ab+");
    if (fp != nullptr) {
        if (_working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || \
	    _working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
	    snprintf(buffer,
		     sizeof(buffer),
		     "frame%08d-l_m_s-gain[%08.5f_%08.5f_%08.5f]-time[%08.5f_%08.5f_%08.5f]-"
		     "awbGain[%08.4f_%08.4f_%08.4f_%08.4f]-dgain[%08d]\n",
		     frame_id,
		     expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
		     expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
		     expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
		     expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.integration_time,
		     expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
		     expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
		     ispParams->data()->awb_gain.rgain,
		     ispParams->data()->awb_gain.grgain,
		     ispParams->data()->awb_gain.gbgain,
		     ispParams->data()->awb_gain.bgain,
		     1);
        } else if (_working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || \
		   _working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
	    snprintf(buffer,
		     sizeof(buffer),
		     "frame%08d-l_s-gain[%08.5f_%08.5f]-time[%08.5f_%08.5f]-"
		     "awbGain[%08.4f_%08.4f_%08.4f_%08.4f]-dgain[%08d]\n",
		     frame_id,
		     expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
		     expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
		     expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.integration_time,
		     expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
		     ispParams->data()->awb_gain.rgain,
		     ispParams->data()->awb_gain.grgain,
		     ispParams->data()->awb_gain.gbgain,
		     ispParams->data()->awb_gain.bgain,
		     1);
	} else {
	    snprintf(buffer,
		     sizeof(buffer),
		     "frame%08d-gain[%08.5f]-time[%08.5f]-"
		     "awbGain[%08.4f_%08.4f_%08.4f_%08.4f]-dgain[%08d]\n",
		     frame_id,
		     expParams->data()->aecExpInfo.LinearExp.exp_real_params.analog_gain,
		     expParams->data()->aecExpInfo.LinearExp.exp_real_params.integration_time,
		     ispParams->data()->awb_gain.rgain,
		     ispParams->data()->awb_gain.grgain,
		     ispParams->data()->awb_gain.gbgain,
		     ispParams->data()->awb_gain.bgain,
		     1);
	}

	fwrite((void *)buffer, strlen(buffer), 1, fp);
	fflush(fp);
	fclose(fp);
    }
}

const struct capture_fmt* Isp20PollThread::find_fmt(const uint32_t pixelformat)
{
    const struct capture_fmt *fmt;
    unsigned int i;

    for (i = 0; i < sizeof(csirx_fmts); i++) {
	fmt = &csirx_fmts[i];
	if (fmt->fourcc == pixelformat)
	    return fmt;
    }

    return NULL;
}

int Isp20PollThread::calculate_stride_per_line(const struct capture_fmt& fmt,
					   uint32_t& bytesPerLine)
{
    uint32_t pixelsPerLine = 0, stridePerLine = 0;
    /* The actual size stored in the memory */
    uint32_t actualBytesPerLine = 0;

    bytesPerLine = sns_width * fmt.bpp[0] / 8;

    pixelsPerLine = fmt.pcpp * DIV_ROUND_UP(sns_width, fmt.pcpp);
    actualBytesPerLine = pixelsPerLine * fmt.bpp[0] / 8;

    /* mipi wc(Word count) must be 4 byte aligned */
    stridePerLine = 4 * DIV_ROUND_UP(actualBytesPerLine, 4);

    LOGD_CAMHW("sns_width: %d, pixelsPerLine: %d, bytesPerLine: %d, stridePerLine: %d\n",
	   sns_width,
	   pixelsPerLine,
	   bytesPerLine,
	   stridePerLine);

    return stridePerLine;
}

/*
 * Refer to "Raw file structure" in the header of this file
 */
XCamReturn
Isp20PollThread::write_frame_header_to_raw(FILE *fp, int dev_index,
					   int sequence)
{
    uint8_t buffer[128] = {0};
    uint32_t stridePerLine = 0, bytesPerLine = 0;
    const struct capture_fmt* fmt = nullptr;
    uint8_t mode = 0;
    uint8_t frame_type = 0, storage_type = 0;

    if (fp == nullptr)
	return XCAM_RETURN_ERROR_PARAM;

    if ((fmt = find_fmt(pixelformat)))
        stridePerLine = calculate_stride_per_line(*fmt, bytesPerLine);

    if (_working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || \
	 _working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
	mode = 3;
	frame_type = dev_index == 0 ? 1: dev_index == 1 ? 2 : 3;
    } else if (_working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || \
	     _working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
	mode = 2;
	frame_type = dev_index == 0 ? 1: 3;
    } else {
	mode = 1;
    }


    *((uint16_t* )buffer) = RAW_FILE_IDENT;	  // Identifier
    *((uint16_t* )(buffer + 2)) = HEADER_LEN;	  // Header length
    *((uint32_t* )(buffer + 4)) = sequence;	  // Frame number
    *((uint16_t* )(buffer + 8)) = sns_width;	  // Image width
    *((uint16_t* )(buffer + 10)) = sns_height;    // Image height
    *(buffer + 12) = fmt->bpp[0];		  // Bit depth
    *(buffer + 13) = fmt->bayer_fmt;		  // Bayer format
    *(buffer + 14) = mode;			  // Number of HDR frame
    *(buffer + 15) = frame_type;		  // Current frame type
    *(buffer + 16) = storage_type;		  // Storage type
    *((uint16_t* )(buffer + 17)) = stridePerLine; // Line stride
    *((uint16_t* )(buffer + 19)) = bytesPerLine;  // Effective line stride

    fwrite(buffer, sizeof(buffer), 1, fp);
    fflush(fp);

    LOGV_CAMHW("frame%d: image rect: %dx%d, %d bit depth, Bayer fmt: %d, "
	   "hdr frame number: %d, frame type: %d, Storage type: %d, "
	   "line stride: %d, Effective line stride: %d\n",
	   sequence, sns_width, sns_height,
	   fmt->bpp[0], fmt->bayer_fmt, mode,
	   frame_type, storage_type, stridePerLine,
	   bytesPerLine);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
Isp20PollThread::write_raw_to_file(FILE* fp, int dev_index,
				   int sequence, void* userptr, int size)
{
    if (fp == nullptr)
	return XCAM_RETURN_ERROR_PARAM;

    fwrite(userptr, size, 1, fp);
    fflush(fp);

    if (!dev_index) {
        for (int i = 0; i < _capture_raw_num; i++)
            printf(">");
	printf("\n");

	LOGV_CAMHW("write frame%d raw\n", sequence);
    }

    return XCAM_RETURN_NO_ERROR;
}

void
Isp20PollThread::write_reg_to_file(uint32_t base_addr, uint32_t offset_addr,
				   int len, int sequence)
{
    char cmd_buffer[128] = {0};

#if 1
    snprintf(cmd_buffer, sizeof(cmd_buffer),
	     "io -4 -l 0x%x 0x%x > %s/frame%d_isp_reg",
	     len,
	     base_addr + offset_addr,
	     raw_dir_path,
	     sequence);
#else
    snprintf(cmd_buffer, sizeof(cmd_buffer),
	     "io -4 -r -f %s/frame%d_isp_reg -l 0x%x 0x%x",
	     raw_dir_path,
	     sequence,
	     len,
	     base_addr + offset_addr);
#endif

    system(cmd_buffer);
}

XCamReturn
Isp20PollThread::creat_raw_dir(const char* path)
{
    time_t now;
    struct tm* timenow;

    if (!path)
	return XCAM_RETURN_ERROR_FAILED;

    time(&now);
    timenow = localtime(&now);
    snprintf(raw_dir_path, sizeof(raw_dir_path), "%s/raw_%04d-%02d-%02d_%02d-%02d-%02d",
	     path,
	     timenow->tm_year + 1900,
	     timenow->tm_mon + 1,
	     timenow->tm_mday,
	     timenow->tm_hour,
	     timenow->tm_min,
	     timenow->tm_sec);

    LOGD_CAMHW("mkdir %s for capturing %d frames raw!\n",
	       raw_dir_path, _capture_raw_num);

    if(mkdir(raw_dir_path, 0755) < 0) {
        LOGE_CAMHW("mkdir %s error!!!\n", raw_dir_path);
	return XCAM_RETURN_ERROR_PARAM;
    }

    _is_raw_dir_exist = true;

    return XCAM_RETURN_ERROR_PARAM;
}

}; //namspace RkCam
