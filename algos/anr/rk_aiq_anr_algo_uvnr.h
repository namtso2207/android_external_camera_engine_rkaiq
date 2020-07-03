#ifndef __RKAIQ_UVNR_H_
#define __RKAIQ_UVNR_H_

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "anr/rk_aiq_types_anr_algo_prvt.h"

RKAIQ_BEGIN_DECLARE

ANRresult_t init_uvnr_params(RKAnr_Uvnr_Params_t *pParams, CalibDb_UVNR_t *pCalibdb, int setting_idx);

ANRresult_t select_uvnr_params_by_ISO(RKAnr_Uvnr_Params_t *stRKUVNrParams, RKAnr_Uvnr_Params_Select_t *stRKUVNrParamsSelected, ANRExpInfo_t *pExpInfo);

ANRresult_t uvnr_fix_transfer(RKAnr_Uvnr_Params_Select_t *uvnr, RKAnr_Uvnr_Fix_t *pNrCfg, ANRExpInfo_t *pExpInfo, float gain_ratio, float fStrength);

ANRresult_t uvnr_fix_Printf(RKAnr_Uvnr_Fix_t  * pNrCfg);

ANRresult_t uvnr_get_setting_idx_by_name(CalibDb_UVNR_t *pCalibdb, char *name, int *setting_idx);
ANRresult_t uvnr_config_setting_param(RKAnr_Uvnr_Params_t *pParams, CalibDb_UVNR_t *pCalibdb, char* snr_name);
RKAIQ_END_DECLARE

#endif

