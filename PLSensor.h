/*
 * Copyright (C) 2012 Intel Mobile Communications GmbH
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

#ifndef ANDROID_LIGHT_SENSOR_H
#define ANDROID_LIGHT_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "SensorsDef.h"
#include "SensorBase.h"
#include "InputEventReader.h"

/*****************************************************************************/

#define PL_DEVICE_NAME              "/dev/vsns_misc"
#define PL_DATA_DEVICE_NAME         "/dev/input/event2"

typedef enum
{
    LightSensor   = 0, //SENSOR_TYPE_LIGHT
    ProximitySensor   = 1,    //SENSOR_TYPE_PROXIMITY
    PLSensorNum
}PLSensorList;

typedef struct PLSensor
{
    SensorBase base;   //must be the first one
    
    sensors_data_t              mPendingEvents[PLSensorNum];
    //bool mHasPendingEvent;
    uint32_t                    mPendingMask;
    InputEventCircularReader*   mInputReader;
    
    
}PLSensor;

extern PLSensor* pl_sensor_new();

/*****************************************************************************/

#endif  // ANDROID_LIGHT_SENSOR_H
