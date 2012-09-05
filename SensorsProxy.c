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


#include <linux/input.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cutils/log.h>

#include "SensorsProxy.h"
#include "SensorBase.h"
#include "PLSensor.h"
#if ENABLE_BOSCH_SENSOR
#include "BmcSensor.h"
#endif

//static const char WAKE_MESSAGE = 'W';

enum 
{
    #if ENABLE_BOSCH_SENSOR
        BmcSensorDriver,
    #endif  
        PLSensorDriver ,
        numSensorDrivers,
        //numFds,
};


//static const size_t s_wake_fd_index = numFds - 1;

static SensorsProxy* s_sensor_proxy = NULL;
struct _SensorsProxy
{
    SensorBase* mSensors[numSensorDrivers];
    struct pollfd mPollFds[numSensorDrivers];

};

static int handleToDriver(int handle) 
{
        switch (handle) 
        {
        #if ENABLE_BOSCH_SENSOR
            case ID_A:
            case ID_M:
            case ID_O:
                return BmcSensorDriver;
        #endif
            case ID_P:
            case ID_L:
                return PLSensorDriver;
        }
        return -EINVAL;
}


//hard code to add sensors
static SensorsProxy* sensors_proxy_new()
{
    SensorsProxy* proxy = malloc(sizeof(SensorsProxy));
    
    //int result ;
    #if SENSORS_DEBUG
    LOGD("sensors_proxy_new ");
    #endif
    memset(proxy,0,sizeof(SensorsProxy));

//init bmc sensor
    #if ENABLE_BOSCH_SENSOR
    
    proxy->mSensors[BmcSensorDriver]  = (SensorBase*)bmc_sensor_new();
    proxy->mPollFds[BmcSensorDriver].fd = -1;//sensor_getFd(proxy->mSensors[BmcSensorDriver] );
    proxy->mPollFds[BmcSensorDriver].events = POLLIN;
    proxy->mPollFds[BmcSensorDriver].revents = 0;
    #endif

//init pl sensor    
    proxy->mSensors[PLSensorDriver] = (SensorBase*)pl_sensor_new();
    proxy->mPollFds[PLSensorDriver].fd = -1;//sensor_getFd(proxy->mSensors[PLSensorDriver] );
    proxy->mPollFds[PLSensorDriver].events = POLLIN;
    proxy->mPollFds[PLSensorDriver].revents = 0;

    return proxy;

}

int sensors_proxy_delete(SensorsProxy* proxy)
{
    int i=0;
    if(proxy==NULL)
        return -1;        
    for(i=0;i<numSensorDrivers;i++)
    {
        sensor_delete(proxy->mSensors[i]);
        proxy->mSensors[i] = NULL;
    }
    //close(proxy->mPollFds[s_wake_fd_index].fd);
    //close(proxy->mWritePipeFd);
    free(proxy);
    s_sensor_proxy = NULL;
    return 0;
}


int sensors_proxy_close(SensorsProxy* proxy,int32_t handle)
{
    int index = handleToDriver(handle);
    #if SENSORS_DEBUG
    LOGD("sensors_proxy_close handle=%d",handle);
    #endif
    if(index==-EINVAL)
        return -1;
    return sensor_close(proxy->mSensors[index]);
}

int sensors_proxy_close_all(SensorsProxy* proxy)
{
    int i=0;
    if(proxy==NULL)
        return -1;

    for(i=0;i<numSensorDrivers;i++)
    {
        sensor_close(proxy->mSensors[i]);
    }
    return 0;
}

int sensors_proxy_open_dev(SensorsProxy* proxy,int32_t handle)
{
    int index = handleToDriver(handle);
    if(index==-EINVAL)
        return -1;
    return  sensor_open_dev(proxy->mSensors[index]);

}

native_handle_t* sensors_proxy_open_all_dev(SensorsProxy* proxy)
{
    int i=0;
    native_handle_t* handle =  native_handle_create(2, 0);;

    for(i=0;i<numSensorDrivers;i++)
    {
        handle->data[i]= sensor_open_dev(proxy->mSensors[i]);
        if(handle->data[i]<0)
        {
            native_handle_close(handle);
            native_handle_delete(handle);
            handle = NULL;
            break;
        }
    }
    return handle;
}

native_handle_t* sensors_proxy_set_fds(SensorsProxy* proxy,native_handle_t* handle)
{
    int i=0;
    #if SENSORS_DEBUG
    LOGD("sensors_proxy_set_fds ");
    #endif
    for(i=0;i<numSensorDrivers;i++)
    {
        #if SENSORS_DEBUG
        LOGD("sensors_proxy_set_fds fd=%d",handle->data[i]);
        #endif
        sensor_setFd(proxy->mSensors[i],dup(handle->data[i]));
    }
    return handle;
}



SensorsProxy* sensors_proxy_get_instance()
{

    if(s_sensor_proxy == NULL)
        s_sensor_proxy = sensors_proxy_new();

    return s_sensor_proxy;
}

//only bmc sensor supports delay 
int sensors_proxy_set_delay(SensorsProxy* proxy,int32_t ms)
{
    #if SENSORS_DEBUG
    LOGD("sensors_proxy_set_delay ms=%d",ms);
    #endif
    #if ENABLE_BOSCH_SENSOR
    return sensor_set_delay(proxy->mSensors[BmcSensorDriver],ms);
    #else
    return 0;
    #endif
}

int sensors_proxy_enable(SensorsProxy* proxy,int32_t handle, int enable)
{
    int index = handleToDriver(handle);
    #if SENSORS_DEBUG
    LOGD("sensors_proxy_enable handle=%d, enable=%d",handle,enable);
    #endif
    if(index==-EINVAL)
        return -1;
    return sensor_enable(proxy->mSensors[index],handle,enable);
}

int sensors_proxy_wake(SensorsProxy* proxy)
{
    #if ENABLE_BOSCH_SENSOR
    return  sensor_wake(proxy->mSensors[BmcSensorDriver]);
    #else
    return 0;
    #endif

}


//only read one sensor data, according to the jni code
int sensors_proxy_poll(SensorsProxy* proxy,sensors_data_t* data)
{
    int i=0;
    int num=0;
    int sensor_type = -1;

    for ( i=0 ;  i<numSensorDrivers ; i++) 
    {
        proxy->mPollFds[i].fd=sensor_getFd(proxy->mSensors[i]);
    }
    do 
    {
        
        for ( i=0 ;  i<numSensorDrivers ; i++) 
        {
            SensorBase* sensor = proxy->mSensors[i];
            #if SENSORS_DEBUG
            if(proxy->mPollFds[i].fd<0)
                LOGD("sensors_proxy_poll i=%d,fd=%d,revents=%d",i,proxy->mPollFds[i].fd,proxy->mPollFds[i].revents);
            #endif
            //if ((proxy->mPollFds[i].revents & POLLIN) || (sensor_has_pending_event(sensor))) 

            if ((proxy->mPollFds[i].revents & POLLIN) ) 
            {

                num = sensor_read_event(sensor,data);
                
                if (num>0) 
                {
                    // no more data for this sensor
                    proxy->mPollFds[i].revents = 0;
                    sensor_type = data->sensor;
                    #if SENSORS_DEBUG
                    LOGD("sensors_proxy_poll invoke sensor_read_events num=%d,sensor=%d",num,data->sensor);
                    #endif
                    break;
                }
            }
        }
        if(num == 0 )
        {

            usleep(100000);
            poll(proxy->mPollFds, numSensorDrivers, -1);
        }
        else
            break;    //got a event , break now
        
    } while (num == 0);

    return sensor_type;
}

