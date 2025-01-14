/*
 * Copyright (C) 2015-2017 Intel Corporation
 * Copyright (c) 2017, Fuzhou Rockchip Electronics Co., Ltd
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
 */
#ifndef _AE_STATE_MACHINE_H_
#define _AE_STATE_MACHINE_H_

#include <xcam_std.h>
#include <CameraMetadata.h>
using ::android::hardware::camera::common::V1_0::helper::CameraMetadata;
#include "rkaiq.h"
#include "rkisp1_aiq_types.h"
#include <base/xcam_log.h>

using namespace android;
namespace XCam {
/**
 * \class RkAEModeBase
 *
 * Base class for all the Autoexposure modes as defined by the Android
 * camera device V3.x API.
 * Each mode will follow certain state transitions. See documentation for
 * android.control.aeState
 *
 */
class RkAEModeBase {
public:
    RkAEModeBase();
    virtual ~RkAEModeBase() {};

    virtual XCamReturn processState(const uint8_t &controlMode,
                                  const AeControls &aeControls) = 0;


    virtual XCamReturn processResult(const rk_aiq_ae_results &aeResults,
                                   CameraMetadata &results,
                                   int reqId) = 0;

    void resetState(void);
    uint8_t getState() const { return mCurrentAeState; }
    uint8_t getLockState() const { return mCurrentAeLockState; }
    uint8_t setLockState(uint8_t state)    {mCurrentAeLockState = state; return 0;}
    uint8_t updateStillPreCapreqId(int reqId) { mStilPreCapreqId = reqId; return 0; }
protected:
    void updateResult(int reqId, CameraMetadata &results);
protected:
    AeControls  mLastAeControls;
    uint8_t     mLastControlMode;
    bool        mEvChanged; /**< set and kept to true when ev changes until
                                 converged */

    bool        mLastAeConvergedFlag;
    uint8_t     mAeRunCount;
    uint8_t     mAeConvergedCount;
    uint8_t     mAePreCapCount;
    uint8_t     mCurrentAeState;
    uint8_t     mCurrentAeLockState;
    int mStilPreCapreqId;
};

/**
 * \class RkAEModeAuto
 * Derived class from RkAEModeBase for Auto mode
 *
 */
class RkAEModeAuto: public RkAEModeBase {
public:
    RkAEModeAuto();
    virtual XCamReturn processState(const uint8_t &controlMode,
                                  const AeControls &aeControls);
    virtual XCamReturn processResult(const rk_aiq_ae_results & aeResults,
                                  CameraMetadata& result,
                                  int reqId);
};

/**
 * \class RkAEModeOFF
 * Derived class from RkAEModeBase for OFF mode
 *
 */
class RkAEModeOff: public RkAEModeBase {
public:
    RkAEModeOff();
    virtual XCamReturn processState(const uint8_t &controlMode,
                                  const AeControls &aeControls);
    virtual XCamReturn processResult(const rk_aiq_ae_results & aeResults,
                                  CameraMetadata& result,
                                  int reqId);
};

/**
 * \class RkAEStateMachine
 *
 * This class adapts the Android V3 AE triggers and state transitions to
 * the ones implemented by the Rockchip AIQ algorithm
 * This class is platform independent. Platform specific behaviors should be
 * implemented in derived classes from this one or from the RkAEModeBase
 *
 */
class RkAEStateMachine {
public:
    RkAEStateMachine();
    virtual ~RkAEStateMachine();

    virtual XCamReturn processState(const uint8_t &controlMode,
                                  const AeControls &aeControls,
                                  int reqId);

    virtual XCamReturn processResult(const rk_aiq_ae_results &aeResults,
                                   CameraMetadata &results,
                                   int reqId);

    uint8_t getState() const { return mCurrentAeMode->getState(); }
    uint8_t getLockState() const { return mCurrentAeMode->getLockState(); }
    uint8_t setLockState(uint8_t state)    { return mCurrentAeMode->setLockState(state); }

private:
    // prevent copy constructor and assignment operator
    RkAEStateMachine(const RkAEStateMachine &other);
    RkAEStateMachine& operator=(const RkAEStateMachine &other);

private: /* members*/
    AeControls  mLastAeControls;
    uint8_t     mLastControlMode;
    uint8_t     mCurrentAeState;
    RkAEModeBase *mCurrentAeMode;

    RkAEModeOff mOffMode;
    RkAEModeAuto mAutoMode;
    int mStilPreCapreqId;
};

}
#endif // _AE_STATE_MACHINE_H_
