/*
 * rk_aiq_types_acp_algo.h
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

#ifndef _RK_AIQ_TYPES_ACP_ALGO_H_
#define _RK_AIQ_TYPES_ACP_ALGO_H_ 

typedef struct rk_aiq_ranges_s {
    float min;
    float max;
} rk_aiq_ranges_t;

typedef struct rk_aiq_acp_params_s {
    /*range:  0.0  - 1.992*/
    float contrast;
    /* -90 deg - +87.188*/
    float hue;
    /*range : -128  - +127 */
    signed char brightness;
    /*range:  0.0  - 1.992*/
    float saturation;
} rk_aiq_acp_params_t;

#endif