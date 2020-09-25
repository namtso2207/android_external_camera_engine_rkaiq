
#ifndef _AIQ_CAMERA_HAL_ADAPTER_H_
#define _AIQ_CAMERA_HAL_ADAPTER_H_

#include "RkAiqCore.h"
#include <RkAiqManager.h>

#include "rkisp_control_loop.h"
#include "rkaiq.h"

#ifdef ANDROID_VERSION_ABOVE_8_X
#include <CameraMetadata.h>
using ::android::hardware::camera::common::V1_0::helper::CameraMetadata;
#include <cutils/properties.h>
#define CAM_IQ_PROPERTY_KEY  "vendor.cam.librkisp.iqfile.name"
#else
#include <camera/CameraMetadata.h>
#endif
#include <pthread.h>

#include "include/ae_state_machine.h"
#include "include/af_state_machine.h"
#include "include/awb_state_machine.h"

#include "settings_processor.h"

/*
 ***************** AIQ ADAPTER LIB VERSION NOTE *****************
* v1.0.0 init
*/

#define CONFIG_AIQ_ADAPTER_LIB_VERSION "v1.0.0"

using namespace RkCam;
using namespace XCam;

typedef struct rk_aiq_sys_ctx_s {
    const char* _sensor_entity_name;
    SmartPtr<RkCam::RkAiqManager> _rkAiqManager;
    SmartPtr<RkCam::ICamHw> _camHw;
    SmartPtr<RkCam::RkAiqCore> _analyzer;
    SmartPtr<RkCam::RkLumaCore> _lumaAnalyzer;
    CamCalibDbContext_t *_calibDb;
} rk_aiq_sys_ctx_t;

struct rkisp_cl_frame_rkaiq_s {
    //frame id
    int id;
    // rkaiq input parameters
    const void *rkaiq;
};

struct rkisp_cl_frame_rkaiq_result_s {
    //frame id
    int id;
    rk_aiq_ae_results  *ae_results;
    rk_aiq_awb_results *awb_results;
    rk_aiq_af_results  *af_results;
};

enum USE_CASE {
  UC_PREVIEW,
  UC_PRE_CAPTRUE,
  UC_CAPTURE,
  UC_RECORDING,
  UC_RAW
};

//namespace RkCam {
class AiqCameraHalAdapter:
    public RkAiqAnalyzerCb,
    public IspStatsListener
{
private:
    SmartPtr<RkAiqManager> _rkAiqManager;
    SmartPtr<RkAiqCore> _analyzer;
    SmartPtr<ICamHw> _camHw;
    SmartPtr<RkAiqAnalyzerCb> _RkAiqAnalyzerCb;
    SmartPtr<IspStatsListener> _IspStatsListener;

    const cl_result_callback_ops_t *mCallbackOps;

    Mutex _settingsMutex;
    // push_back when set_control_params, erase when calculationd done
    std::vector<SmartPtr<AiqInputParams>>  _settings;
    std::vector<SmartPtr<AiqInputParams>>  _fly_settings;
    SmartPtr<AiqInputParams>  _cur_settings;
    SettingsProcessor*            _settingsProcessor;
    SmartPtr<AiqInputParams> _inputParams;
    SmartPtr<RkAEStateMachine>     mAeState;      /**< AE state machine */
    SmartPtr<RkAFStateMachine>      mAfState;     /**< AF state machine */
    SmartPtr<RkAWBStateMachine>    mAwbState;     /**< AWB state machine */
    rk_aiq_exposure_sensor_descriptor _sns_des;

    /* MetaData*/
    CameraMetadata *_metadata;
    camera_metadata_t *_meta;
    //TODO for Capture or Flash
    bool _delay_still_capture;
    rk_aiq_working_mode_t _work_mode;

public:
    rk_aiq_sys_ctx_t* _aiq_ctx;
    static CameraMetadata staticMeta;

public:
    AiqCameraHalAdapter(SmartPtr<RkAiqManager> _rkAiqManager,SmartPtr<RkAiqCore> _analyzer,SmartPtr<ICamHw> _camHw);
    ~AiqCameraHalAdapter();
    void init(const cl_result_callback_ops_t* callbacks);
    void processResults(SmartPtr<RkAiqFullParamsProxy> &results);
    XCamReturn getAeResults(rk_aiq_ae_results &ae_results);
    void getAeResultsDebug(rk_aiq_ae_results &ae_results, SmartPtr<rk_aiq_exposure_params_wrapper_t> exp_param);
    XCamReturn getAfResults(rk_aiq_af_results &af_results, SmartPtr<rk_aiq_focus_params_t> focus_param);
    void getAfResultsDebug(rk_aiq_af_results &af_results, SmartPtr<rk_aiq_focus_params_t> focus_param);
    XCamReturn getAwbResults(rk_aiq_awb_results &awb_results);
    void getAwbResultsDebug(rk_aiq_awb_results &awb_results, SmartPtr<rk_aiq_isp_params_t> isp_param);
    void updateParams(SmartPtr<VideoBuffer>& ispStats);
    void updateAeParams(XCamAeParam *aeParams);
    void updateAwbParams(XCamAwbParam *awbParams);
    void updateAfParams(XCamAfParam *afParams);
    void setFrameParams(const struct rkisp_cl_frame_rkaiq_s* frame_params);

    virtual void rkAiqCalcDone(SmartPtr<RkAiqFullParamsProxy> &results);
    virtual void rkAiqCalcFailed(const char* msg);

    virtual XCamReturn ispStatsCb(SmartPtr<VideoBuffer>& ispStats);

    SmartPtr<AiqInputParams> getAiqInputParams();
    void set_aiq_ctx(rk_aiq_sys_ctx_t* aiq_ctx) { _aiq_ctx = aiq_ctx; };
    rk_aiq_sys_ctx_t* get_aiq_ctx() { return _aiq_ctx; };
    void set_static_metadata(const camera_metadata_t *metas) { staticMeta = metas; };
    static CameraMetadata& get_static_metadata() { return staticMeta; };
    XCamReturn set_control_params(const int request_frame_id,
                                  const camera_metadata_t *metas);
    void setAiqInputParams(SmartPtr<AiqInputParams> inputParams) { _inputParams  = inputParams; };
    SmartPtr<AiqInputParams> getAiqInputParams_simple() { return _inputParams; };

    void updateMetaParams(SmartPtr<VideoBuffer>& ispStats);
    void updateAeMetaParams(XCamAeParam *aeParams);
    void updateAfMetaParams(XCamAfParam *afParams);
    void updateAwbMetaParams(XCamAwbParam *awbParams);
    void updateOtherMetaParams();

    void pre_process_3A_states(SmartPtr<AiqInputParams> inputParams);
    bool set_sensor_mode_data (rk_aiq_exposure_sensor_descriptor *sensor_mode,
                               bool first);
    XCamReturn processAeMetaResults(rk_aiq_ae_results &ae_result, CameraMetadata *metadata);
    XCamReturn processAfMetaResults(rk_aiq_af_results &af_result, CameraMetadata *metadata);
    XCamReturn processAwbMetaResults(rk_aiq_awb_results &awb_result, CameraMetadata *metadata);
    XCamReturn processMiscMetaResults(CameraMetadata *metadata);
    void set_working_mode(rk_aiq_working_mode_t work_mode) { _work_mode = work_mode; };

};
//}; //namespace RkCam


#endif //_AIQ_CAMERA_HAL_ADAPTER_H_
