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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <sys/time.h>
//#include <time.h>
#include <cutils/log.h>

#include <linux/input.h>


#include "SensorBase.h"
#include "SensorsDef.h"

#define SENSOR_INPUT_DEV "/dev/input/event2"
/*****************************************************************************/


void sensor_init(SensorBase* sensor)
{
    sensor->data_fd = -1;
}

void sensor_deinit(SensorBase* sensor)
{
    if (sensor->data_fd >= 0)
    {
        close(sensor->data_fd);
        sensor->data_fd = -1;
    }
}
int sensor_open_dev(SensorBase* sensor)
{
    if(sensor->open_dev)
		return sensor->open_dev(sensor);
	return -1;
}

int sensor_close(SensorBase* sensor)
{
    if(sensor->close)
		return sensor->close(sensor);
	return 0;
}
int sensor_delete(SensorBase* sensor)
{
    if(sensor->delete)
        return sensor->delete(sensor);
    return 0;
}

int sensor_read_event(SensorBase* sensor,sensors_data_t* data)
{
	if(sensor->read_event)
		return sensor->read_event(sensor,data);
	return 0;
}


int sensor_getFd(SensorBase* sensor)
{
    if(sensor->getFd)
	    return sensor->getFd(sensor);
    return sensor->data_fd;
}

int sensor_setFd(SensorBase* sensor,int fd)
{
	#if SENSORS_DEBUG
    LOGD("sensor_setFd fd=%d",fd);
    #endif
    if(sensor->setFd)
	    return sensor->setFd(sensor,fd);
    sensor->data_fd = fd;
    return 0;
}

int  sensor_set_delay(SensorBase* sensor , int32_t ms)
{
	if(sensor->set_delay)
		return sensor->set_delay(sensor,ms);
	return 0;
}

int sensor_enable(SensorBase* sensor ,int32_t handle, int enabled)
{
	if(sensor->enable)
		return sensor->enable(sensor,handle,enabled);
	return 0;
}

int sensor_is_enable(SensorBase* sensor)
{
    if(sensor->is_enable)
        return sensor->is_enable(sensor);
    return 0;
}

int sensor_wake(SensorBase* sensor)
{
	int err = 0;
	int fd = open(SENSOR_INPUT_DEV, O_WRONLY);
	UNUSED(sensor);
	if (fd > 0) 
    {
		struct input_event event[1];
		event[0].type = EV_SYN;
		event[0].code = SYN_CONFIG;
		event[0].value = 0;
		err = write(fd, event, sizeof(event));
        if(err < 0)
		    LOGD( "sensor_wake, err=%d (%s)", errno, strerror(errno));
		close(fd);
	}
	return err;
}

int64_t get_time_stamp() 
{
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (t.tv_sec)*1000000000LL + t.tv_nsec;
}

int64_t timevalToNano(struct timeval* t) 
{ 
	return t->tv_sec*1000000000LL + t->tv_usec*1000;
}



