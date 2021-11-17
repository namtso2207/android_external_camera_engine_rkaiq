/*
 * rk_aiq_algo_camgroup_amerge_itf.c
 *
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

#include "rk_aiq_algo_types_int.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "xcam_log.h"
#include "algos/amerge/rk_aiq_amerge_algo.h"
#include "algos/amerge/rk_aiq_types_amerge_algo_prvt.h"



RKAIQ_BEGIN_DECLARE


static XCamReturn AmergeCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AlgoCtxInstanceCfgCamGroup* instanc_int = (AlgoCtxInstanceCfgCamGroup*)cfg;
    AmergeContext_t* pAmergeGrpCtx = NULL;

    ret = AmergeInit(&pAmergeGrpCtx, (CamCalibDbV2Context_t*)(instanc_int->s_calibv2));

    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_AMERGE("%s Amerge Init failed: %d", __FUNCTION__, ret);
        return(XCAM_RETURN_ERROR_FAILED);
    }
    *context = (RkAiqAlgoContext *)(pAmergeGrpCtx);

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(ret);
}

static XCamReturn AmergeDestroyCtx(RkAiqAlgoContext *context)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(context != NULL) {
        AmergeContext_t* pAmergeGrpCtx = (AmergeContext_t*)context;
        ret = AmergeRelease(pAmergeGrpCtx);
        if (ret != AMERGE_RET_SUCCESS) {
            LOGE_AMERGE("%s Amerge Release failed: %d", __FUNCTION__, ret);
            return(XCAM_RETURN_ERROR_FAILED);
        }
        context = NULL;
    }

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(ret);
}

static XCamReturn AmergePrepare(RkAiqAlgoCom* params)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AmergeContext_t* pAmergeGrpCtx = (AmergeContext_t*)params->ctx;
    RkAiqAlgoCamGroupPrepare* pAmergePrepParams = (RkAiqAlgoCamGroupPrepare*)params; //come from params in html
    const CamCalibDbV2Context_t* pCalibDb = pAmergePrepParams->s_calibv2;

    if (pAmergePrepParams->gcom.com.u.prepare.working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAmergeGrpCtx->FrameNumber = 1;
    else if (pAmergePrepParams->gcom.com.u.prepare.working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             pAmergePrepParams->gcom.com.u.prepare.working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAmergeGrpCtx->FrameNumber = 2;
    else
        pAmergeGrpCtx->FrameNumber = 3;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        LOGD_AMERGE("%s: Amerge Reload Para!\n", __FUNCTION__);
        if(pAmergeGrpCtx->HWversion == AMERGE_ISP20 || pAmergeGrpCtx->HWversion == AMERGE_ISP21) {
            CalibDbV2_merge_t* calibv2_amerge_calib =
                (CalibDbV2_merge_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, amerge_calib));
            memcpy(&pAmergeGrpCtx->pCalibDB.Merge_v20, calibv2_amerge_calib, sizeof(CalibDbV2_merge_t));//load iq paras
        }
        else if(pAmergeGrpCtx->HWversion == AMERGE_ISP30) {
            CalibDbV2_merge_V2_t* calibv2_amerge_calib =
                (CalibDbV2_merge_V2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, amerge_calib));
            memcpy(&pAmergeGrpCtx->pCalibDB.Merge_v30, calibv2_amerge_calib, sizeof(CalibDbV2_merge_V2_t));//load iq paras

        }
    }

    if(/* !params->u.prepare.reconfig*/true) {
        AmergeStop(pAmergeGrpCtx); // stop firstly for re-preapre
        ret = AmergeStart(pAmergeGrpCtx);
        if (ret != AMERGE_RET_SUCCESS) {
            LOGE_AMERGE("%s Amerge Start failed: %d\n", __FUNCTION__, ret);
            return(XCAM_RETURN_ERROR_FAILED);
        }
    }

    if(pAmergeGrpCtx->FrameNumber == 2 || pAmergeGrpCtx->FrameNumber == 3) {
        if(pAmergeGrpCtx->mergeAttr.opMode == MERGE_OPMODE_TOOL) {
            //MergeNewMalloc(&pAmergeGrpCtx->Config, &pAmergeGrpCtx->mergeAttr.stTool, pAmergeGrpCtx->HWversion);
            //AmergeUpdateConfig(pAmergeGrpCtx, &pAmergeGrpCtx->mergeAttr.stTool);
        }
        else {
            MergeNewMalloc(&pAmergeGrpCtx->Config, &pAmergeGrpCtx->pCalibDB, pAmergeGrpCtx->HWversion);
            AmergeUpdateConfig(pAmergeGrpCtx, &pAmergeGrpCtx->pCalibDB);
        }
    }

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(ret);
}

static XCamReturn AmergeProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    bool bypass = false;

    AmergeContext_t* pAmergeGrpCtx = (AmergeContext_t*)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* pAmergeGrpParams = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* pAmergeGrpProcRes = (RkAiqAlgoCamGroupProcOut*)outparams;

    if(pAmergeGrpCtx->FrameNumber == 2 || pAmergeGrpCtx->FrameNumber == 3) {
        LOGD_AMERGE("/#####################################Amerge Group Start#####################################/ \n");
        //get Sensor Info
        XCamVideoBuffer* xCamAeProcRes = pAmergeGrpParams->camgroupParmasArray[0]->aec._aeProcRes;
        RkAiqAlgoProcResAeInt* pAEProcRes = NULL;
        if (xCamAeProcRes) {
            pAEProcRes = (RkAiqAlgoProcResAeInt*)xCamAeProcRes->map(xCamAeProcRes);
            AmergeGetSensorInfo(pAmergeGrpCtx, pAEProcRes->ae_proc_res_rk);
        }
        else {
            AecProcResult_t AeProcResult;
            memset(&AeProcResult, 0x0, sizeof(AecProcResult_t));
            LOGE_AMERGE("%s: Ae Proc result is null!!!\n", __FUNCTION__);
            AmergeGetSensorInfo(pAmergeGrpCtx, AeProcResult);
        }

        //get ae pre res and proc
        XCamVideoBuffer* xCamAePreRes = pAmergeGrpParams->camgroupParmasArray[0]->aec._aePreRes;
        RkAiqAlgoPreResAeInt* pAEPreRes = NULL;
        if (xCamAePreRes) {
            pAEPreRes = (RkAiqAlgoPreResAeInt*)xCamAePreRes->map(xCamAePreRes);
            bypass = AmergeByPassProcessing(pAmergeGrpCtx, pAEPreRes->ae_pre_res_rk);
        }
        else {
            AecPreResult_t AecHdrPreResult;
            memset(&AecHdrPreResult, 0x0, sizeof(AecPreResult_t));
            bypass = AmergeByPassProcessing(pAmergeGrpCtx, AecHdrPreResult);
            bypass = false;
            LOGE_AMERGE("%s: ae Pre result is null!!!\n", __FUNCTION__);
        }

        if(!bypass)
            AmergeProcessing(pAmergeGrpCtx);

        pAmergeGrpCtx->PrevData.CtrlData.ApiMode = pAmergeGrpCtx->mergeAttr.opMode;
        pAmergeGrpCtx->ProcRes.update = !bypass;
        pAmergeGrpCtx->ProcRes.LongFrameMode = pAmergeGrpCtx->SensorInfo.LongFrmMode;
        for(int i = 0; i < pAmergeGrpProcRes->arraySize; i++) {
            pAmergeGrpProcRes->camgroupParmasArray[i]->_amergeConfig->update = pAmergeGrpCtx->ProcRes.update;
            pAmergeGrpProcRes->camgroupParmasArray[i]->_amergeConfig->LongFrameMode = pAmergeGrpCtx->ProcRes.LongFrameMode;
            if(pAmergeGrpCtx->HWversion == AMERGE_ISP20 || pAmergeGrpCtx->HWversion == AMERGE_ISP21)
                memcpy(&pAmergeGrpProcRes->camgroupParmasArray[i]->_amergeConfig->Merge_v20, &pAmergeGrpCtx->ProcRes.Merge_v20, sizeof(MgeProcRes_t));
            else if(pAmergeGrpCtx->HWversion == AMERGE_ISP30)
                memcpy(&pAmergeGrpProcRes->camgroupParmasArray[i]->_amergeConfig->Merge_v30, &pAmergeGrpCtx->ProcRes.Merge_v30, sizeof(MgeProcResV2_t));
        }

        LOGD_AMERGE("/#####################################Amerge Group Over#####################################/ \n");
    }

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(ret);
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAmerge = {
    .common = {
        .version = RKISP_ALGO_AMERGE_VERSION,
        .vendor  = RKISP_ALGO_AMERGE_VENDOR,
        .description = RKISP_ALGO_AMERGE_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AMERGE,
        .id      = 0,
        .create_context  = AmergeCreateCtx,
        .destroy_context = AmergeDestroyCtx,
    },
    .prepare = AmergePrepare,
    .pre_process = NULL,
    .processing = AmergeProcess,
    .post_process = NULL,
};

RKAIQ_END_DECLARE