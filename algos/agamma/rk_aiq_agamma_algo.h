/*
 * rk_aiq_agamma_algo.h
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

#ifndef __RK_AIQ_AGAMMA_ALGO_H__
#define __K_AIQ_AGAMMA_ALGO_H__

#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "agamma/rk_aiq_types_agamma_algo_prvt.h"


RKAIQ_BEGIN_DECLARE

  XCamReturn AgammaInitV200(AgammaHandle_t** para);
XCamReturn AgammaReleaseV200(AgammaHandle_t* para);
XCamReturn AgammaConfigV200(const CalibDb_Gamma_t *  AgammaHtmlPara, AgammaHandle_t* para);
XCamReturn AgammaPreProcV200(AgammaHandle_t* para);
XCamReturn AgammaProcessingV200(AgammaHandle_t* para);
XCamReturn  AgammaReConfigV200(AgammaHandle_t *para,const CalibDb_Gamma_t * gamma_calib);

RKAIQ_END_DECLARE

#endif //_RK_AIQ_ALGO_ACPRC_ITF_H_