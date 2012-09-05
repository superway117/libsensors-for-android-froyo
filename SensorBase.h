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

#ifndef ANDROID_SENSOR_BASE_H
#define ANDROID_SENSOR_BASE_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/time.h>

#include <hardware/sensors.h>
/*****************************************************************************/




struct SensorBase;
typedef struct SensorBase SensorBase;

struct SensorBase
{
    //const char* dev_name;
    //const char* data_name;
    //int         dev_fd;
    int         data_fd;
    
    int (*delete)(SensorBase* sensor);

    int (*open_dev)(SensorBase* sensor);        //just open data device, return fd handle,the fd do not remain inside sensor context

    int (*close)(SensorBase* sensor);           //

    int (*read_event)(SensorBase* sensor,sensors_data_t* data);

    int (*getFd)(SensorBase* sensor);

    int (*setFd)(SensorBase* sensor,int fd);

    int (*set_delay)(SensorBase* sensor , int32_t ms);

    int (*enable)(SensorBase* sensor ,int32_t handle, int enabled);

    int (*is_enable)(SensorBase* sensor);
    
};


extern void sensor_init(SensorBase* sensor);

extern void sensor_deinit(SensorBase* sensor);

extern int sensor_delete(SensorBase* sensor);

extern int sensor_open_dev(SensorBase* sensor);

extern int sensor_close(SensorBase* sensor);

extern int sensor_read_event(SensorBase* sensor,sensors_data_t* data);

extern int sensor_getFd(SensorBase* sensor);

extern int sensor_setFd(SensorBase* sensor,int fd);


extern int sensor_set_delay(SensorBase* sensor , int32_t ms);

extern int sensor_enable(SensorBase* sensor ,int32_t handle, int enabled);

extern int sensor_is_enable(SensorBase* sensor);

extern int sensor_wake(SensorBase* sensor);

extern int64_t get_time_stamp();

extern int64_t timevalToNano(struct timeval *t);




/*****************************************************************************/

#endif  // ANDROID_SENSOR_BASE_H
