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

#include "rk_aiq_user_api_acp.h"
#include "RkAiqHandleInt.h"

RKAIQ_BEGIN_DECLARE

XCamReturn  rk_aiq_user_api_acp_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, acp_attrib_t attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ACP);

    RkAiqAcpHandleInt* algo_handle =
        algoHandle<RkAiqAcpHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACP);

    if (algo_handle) {
        return algo_handle->setAttrib(attr);
    }

    return (ret);
}

XCamReturn  rk_aiq_user_api_acp_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, acp_attrib_t *attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAcpHandleInt* algo_handle =
        algoHandle<RkAiqAcpHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACP);

    if (algo_handle) {
        return algo_handle->getAttrib(attr);
    }

    return (ret);

}

RKAIQ_END_DECLARE


