/******************************************************************************
 *
 * Copyright 2019, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/
#ifndef __RK_AIQ_AHDR_ALGO_H__
#define __RK_AIQ_AHDR_ALGO_H__

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "ae/rk_aiq_types_ae_algo_int.h"
#include "af/rk_aiq_af_hw_v200.h"
#include "rk_aiq_uapi_ahdr_int.h"
#include "rk_aiq_types_ahdr_algo_prvt.h"


RESULT AhdrStart(AhdrHandle_t pAhdrCtx);
RESULT AhdrStop(AhdrHandle_t pAhdrCtx);
void AhdrConfig(AhdrHandle_t pAhdrCtx);
void AhdrGetROData(AhdrHandle_t pAhdrCtx, rkisp_ahdr_stats_t* ROData) ;
void AhdrGetXmlParas(AhdrHandle_t pAhdrCtx, const CamCalibDbContext_t* pConfig);
void AhdrUpdateConfig(AhdrHandle_t pAhdrCtx, AecPreResult_t AecHdrPreResult, af_preprocess_result_t AfPreResult);
RESULT AhdrInit(AhdrInstanceConfig_t* pInstConfig) ;
RESULT AhdrRelease(AhdrHandle_t pAhdrCtx) ;
void AhdrProcessing(AhdrHandle_t pAhdrCtx);


#endif
