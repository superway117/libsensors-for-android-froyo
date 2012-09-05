/*
 * Copyright (C) 2012 Intel Mobile Communications GmbH
 * 
 * Copyright (C) 2011 Bosch Sensortec GmbH
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

#if ENABLE_BOSCH_SENSOR
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <time.h>


#include <linux/fs.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>


#include <cutils/log.h>

/*****************************************************************************/


#include "BmcSensor.h"

static int bmc_sensor_enable(SensorBase* sensor,int32_t handle, int enable);


static int bmc_sensor_init_channel(const char* filename)
{
    int err = 0;


    err = access(filename, F_OK | R_OK | W_OK);
    if (err)
    {
        #if SENSORS_DEBUG
        LOGE("init_channels can not access %s, will be created", filename);
        #endif 
        err = mkfifo(filename, 0666);
        if (err)
        {
            LOGE("init_channels error creating file: %s,err=%d,err=%s", filename,err,strerror(-err));
            return err;
        }
    }
    return err;
}

static int bmc_sensor_open_channel(const char* filename)
{
    int err = 0;
    int fd = -1;

    err = bmc_sensor_init_channel(filename);
    if(err)
        return err;

    fd = open(filename, O_RDWR);
    if (fd < 0)
    {

        LOGE("init_channels error openning file: %s", filename);
        return fd;
    }

    return fd;
}

static int init_channels()
{
    bmc_sensor_init_channel(FIFO_DAT);
    return bmc_sensor_init_channel(FIFO_CMD);
}


static int handle2id(int32_t handle)
{
    int id = -1;
    switch (handle)
    {
    case ID_A:
        id = SENSOR_HANDLE_ACCELERATION;
        break;
    case ID_M:
        id = SENSOR_HANDLE_MAGNETIC_FIELD;
        break;
    case ID_O:
        id = SENSOR_HANDLE_ORIENTATION;
        break;
    }

    return id;
}


static int bmc_sensor_open_dev(SensorBase* sensor)
{
    UNUSED(sensor);
    return bmc_sensor_open_channel(FIFO_DAT);
}



static int bmc_sensor_close(SensorBase* sensor)
{
    BmcSensor* bmc_sensor = (BmcSensor*)sensor;
    sensor_deinit(&bmc_sensor->base);
    return 0;
}
static int bmc_sensor_delete(SensorBase* sensor)
{
    BmcSensor* bmc_sensor = (BmcSensor*)sensor;
    bmc_sensor_close(sensor);
    free(bmc_sensor);

    return 0;
}



static int bmc_sensor_enable(SensorBase* sensor,int32_t handle, int enable)
{
    //BmcSensor* bmc_sensor = (BmcSensor*)sensor;
    int err = 0;
    int id = -1;
    struct exchange cmd;
    //int pos = (int)handle;
    int cmd_fd = bmc_sensor_open_channel(FIFO_CMD);
    UNUSED(sensor);

    id = handle2id(handle);
    #if SENSORS_DEBUG
    LOGE("bmc_sensor_enable code=%d,cmd=%d,value=%d,handle=%d",id,SET_SENSOR_ACTIVE,enable,handle);
    #endif
    if (-1 == id || cmd_fd < 0)
    {
        return -EINVAL;
    }

    enable = !!enable;

    cmd.magic = CHANNEL_PKT_MAGIC_CMD;

    cmd.command.cmd = SET_SENSOR_ACTIVE;
    cmd.command.code = id;
    cmd.command.value = enable;
    
    err = write(cmd_fd, &cmd, sizeof(cmd));
    
    if(err<=0)
        LOGE("bmc_sensor_enable err=%d, %s\n", err,strerror(errno));
    
    err = err < (int)sizeof(cmd) ? -1 : 0;
    
    close(cmd_fd);
    return err;
}

static int bmc_sensor_set_delay(SensorBase* sensor, int32_t ms)
{
    int err = 0;
    int id = -1;
    struct exchange cmd;
    int cmd_fd = bmc_sensor_open_channel(FIFO_CMD);
    int32_t handle = ID_A;   //set ID_A AND ID_M delay
    UNUSED(sensor);
    id = handle2id(handle);
    #if SENSORS_DEBUG
    LOGE("bmc_sensor_set_delay code=%d,cmd=%d,value=%d,handle=%d",id,SET_SENSOR_DELAY,ms,handle);
    #endif

    if (-1 == id || cmd_fd<0)
    {
        return -EINVAL;
    }


    cmd.magic = CHANNEL_PKT_MAGIC_CMD;

    cmd.command.cmd = SET_SENSOR_DELAY;
    cmd.command.code = id;
    cmd.command.value = ms ;

    err = write(cmd_fd, &cmd, sizeof(cmd));

    //resend the command for M SENSOR   
    handle = ID_M; 
    id = handle2id(handle);
    cmd.command.code = id;
    err = write(cmd_fd, &cmd, sizeof(cmd));

    if(err<=0)
        LOGE("bmc_sensor_enable err=%d, %s\n", err,strerror(errno));
    err = err < (int)sizeof(cmd) ? -1 : 0;
    close(cmd_fd);
    return err;
}


static int bmc_sensor_read_event(SensorBase* sensor,sensors_data_t *pdata)
{
    BmcSensor* bmc_sensor = (BmcSensor*)sensor;
    int err;
    struct timespec ts;
    int64_t time_ns;
    struct exchange sensor_data;
    sensors_data_t *pdata_cur;

    if( bmc_sensor->base.data_fd<0)  //need check again
    {
        #if SENSORS_DEBUG
        LOGE("bmc_sensor_read_events mEnabled=%d,data_fd=%d",0,bmc_sensor->base.data_fd);
        #endif
        return 0;
    }
    pdata_cur = pdata;
 

    err = read(bmc_sensor->base.data_fd, &sensor_data, sizeof(sensor_data));
    if (err < (int)sizeof(sensor_data))
    {
        return 0;
    }

    if (CHANNEL_PKT_MAGIC_DAT != sensor_data.magic)
    {
        LOGE("discard invalid data packet from stream");
        return 0;
    }

    ts.tv_sec = ts.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    time_ns = ts.tv_sec * 1000000000LL + ts.tv_nsec;
    pdata_cur->time = time_ns;

    switch (sensor_data.data.sensor)
    {
    case SENSOR_HANDLE_ACCELERATION:
        pdata_cur->acceleration.x = GRAVITY_EARTH * sensor_data.data.acceleration.x;
        pdata_cur->acceleration.y = GRAVITY_EARTH * sensor_data.data.acceleration.y;
        pdata_cur->acceleration.z = GRAVITY_EARTH * sensor_data.data.acceleration.z;
        pdata_cur->acceleration.status = sensor_data.data.acceleration.status;
        pdata_cur->sensor = ID_A;//SENSOR_TYPE_ACCELEROMETER;
        #if SENSORS_DEBUG
        LOGD("SENSOR_HANDLE_ACCELERATION data x=%f,y=%f,z=%f,status=%d",pdata_cur->acceleration.x,pdata_cur->acceleration.y,pdata_cur->acceleration.z,pdata_cur->acceleration.status);
        #endif
        break;

    case SENSOR_HANDLE_MAGNETIC_FIELD:
        pdata_cur->magnetic.x = sensor_data.data.magnetic.x;
        pdata_cur->magnetic.y = sensor_data.data.magnetic.y;
        pdata_cur->magnetic.z = sensor_data.data.magnetic.z;
        pdata_cur->magnetic.status = sensor_data.data.magnetic.status;
        pdata_cur->sensor = ID_M;//SENSOR_TYPE_MAGNETIC_FIELD;
        #if SENSORS_DEBUG
        LOGD("SENSOR_HANDLE_MAGNETIC_FIELD data x=%f,y=%f,z=%f,status=%d",pdata_cur->magnetic.x,pdata_cur->magnetic.y,pdata_cur->magnetic.z,pdata_cur->magnetic.status);
        #endif
        break;
    case SENSOR_HANDLE_ORIENTATION:
        pdata_cur->orientation.azimuth = sensor_data.data.orientation.azimuth;
        pdata_cur->orientation.pitch = sensor_data.data.orientation.pitch;
        pdata_cur->orientation.roll = sensor_data.data.orientation.roll;
        pdata_cur->orientation.status = sensor_data.data.orientation.status;
        pdata_cur->sensor = ID_O;//SENSOR_TYPE_ORIENTATION;
        #if SENSORS_DEBUG
        LOGD("SENSOR_HANDLE_ORIENTATION data azimuth=%f,azimuth=%f,roll=%f,status=%d",pdata_cur->orientation.azimuth,pdata_cur->orientation.pitch,pdata_cur->orientation.roll,pdata_cur->orientation.status);
        #endif
        break;
    default:
        LOGE("Invalid data pkt");
        return 0;
    }

    #if SENSORS_DEBUG
    LOGI("bmc_sensor_read_events got data a pack \n");
    #endif
    return 1;
}


BmcSensor* bmc_sensor_new()
{
    BmcSensor* sensor = malloc(sizeof(BmcSensor));
    if (sensor==NULL)
        return NULL;
    memset(sensor,0,sizeof(BmcSensor));

    sensor_init(&sensor->base);

    sensor->base.open_dev = bmc_sensor_open_dev;
    sensor->base.close = bmc_sensor_close;
    sensor->base.delete = bmc_sensor_delete;
    sensor->base.read_event = bmc_sensor_read_event;

    sensor->base.set_delay = bmc_sensor_set_delay;
    sensor->base.enable = bmc_sensor_enable;

    init_channels();


    return sensor;


}

#endif