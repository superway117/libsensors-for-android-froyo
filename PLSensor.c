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

#include <linux/lightsensor.h>

#include <cutils/log.h>

#include "PLSensor.h"

/*****************************************************************************/

static int pl_sensor_open_dev()
{
    int fd;
    fd = open(PL_DATA_DEVICE_NAME, O_RDONLY);

    if (fd < 0) 
    {
        LOGE("pl_sensor_open_dev Couldn't open error = %d(%s)",  fd,strerror(errno));
        return fd;
    }

    return fd;
}


static int pl_sensor_close(SensorBase* sensor)
{
    PLSensor* pl_sensor = (PLSensor*)sensor;

    sensor_deinit(&pl_sensor->base);
    return 0;
}
static int pl_sensor_delete(SensorBase* sensor)
{
    PLSensor* pl_sensor = (PLSensor*)sensor;

    pl_sensor_close(sensor);
    event_reader_delete(pl_sensor->mInputReader);
    
    free(pl_sensor);
    return 0;
}



static int pl_sensor_enable(SensorBase* sensor,int32_t handle, int en) 
{
    int flags = en ? 1 : 0;
	int ret=0;
	int ioctl_type = -1;
    int fd;

    UNUSED(sensor);
    #if SENSORS_DEBUG
    LOGD("pl_sensor_enable handle=%d,en=%d", handle,en);
    #endif

    switch (handle) 
	{
        case ID_P: ioctl_type = SENSOR_IOCTL_VPS_SET_ENABLE; break; //SENSOR_IOCTL_VALS_SET_ENABLE
        case ID_L: ioctl_type = SENSOR_IOCTL_VALS_SET_ENABLE; break;   //SENSOR_IOCTL_VPS_SET_ENABLE
    }

    if (ioctl_type==-1)
        return -EINVAL;

	fd = open(PL_DEVICE_NAME, O_RDWR);
    if(fd<0)
    {
        LOGE("pl_sensor_enable open %s err = (%s)", PL_DEVICE_NAME,strerror(fd));
        return -EINVAL;
    }

    ret = ioctl(fd, ioctl_type, flags);

    ret = ret<0 ? -errno : 0;

    #if SENSORS_DEBUG
    if(ret)
	  LOGE("pl_sensor_enable err = (%s)", strerror(-ret));
    #endif
	close(fd);
    
    return ret;
}


static void pl_sensor_process_event(PLSensor* pl_sensor,int code, int value)
{
    
    switch (code) 
	{
        case EVENT_TYPE_LIGHT:
            pl_sensor->mPendingMask |= 1<<LightSensor;
			pl_sensor->mPendingEvents[LightSensor].light = (float)value;
            #if SENSORS_DEBUG
            LOGD("EVENT_TYPE_LIGHT value=%d",value);
            #endif
            break;
        case EVENT_TYPE_PROXIMITY:
            pl_sensor->mPendingMask |= 1<<ProximitySensor;
			pl_sensor->mPendingEvents[ProximitySensor].distance = (float)value;
            #if SENSORS_DEBUG
            LOGD("EVENT_TYPE_PROXIMITY value=%d",value);
            #endif
            break;
    }
}

//static int pl_sensor_read_event(SensorBase* sensor,sensors_data_t* data, int count)
static int pl_sensor_read_event(SensorBase* sensor,sensors_data_t* data)
{
    PLSensor* pl_sensor = (PLSensor*)sensor;
    int numEventReceived = 0;
    struct input_event* event;
    ssize_t n ;

    if(pl_sensor->base.data_fd <0)
        return -1;
    
    n = event_reader_fill(pl_sensor->mInputReader,pl_sensor->base.data_fd);
    if (n <= 0)
        return n;

    
    while (numEventReceived==0 && event_reader_read(pl_sensor->mInputReader,&event)) 
	{
        int type = event->type;
        LOGI("pl_sensor_read_events type=%d,code=%d\n",type,event->code);
        if (type == EV_ABS) 
		{
			pl_sensor_process_event(pl_sensor,event->code, event->value);
			event_reader_next(pl_sensor->mInputReader);
        } 
		else if(type == EV_SYN)
		{
            int64_t time = timevalToNano(&event->time);
            int j=0;
            for (j=0 ; pl_sensor->mPendingMask && j<PLSensorNum ; j++) 
			{
                if (pl_sensor->mPendingMask & (1<<j)) 
				{
                    pl_sensor->mPendingMask &= ~(1<<j);
                    pl_sensor->mPendingEvents[j].time = time;
                    //if (pl_sensor->mEnabled & (1<<j)) 
                    if ((1<<j)) 
					{
                        *data++ = pl_sensor->mPendingEvents[j];
                        numEventReceived++;
                    }
                }
            }
            if (!pl_sensor->mPendingMask) 
            {
                event_reader_next(pl_sensor->mInputReader);
            }
        }
		
		else 
		{
            #if SENSORS_DEBUG
            LOGE("PLSensor: unknown event (type=%d, code=%d)",type, event->code);
            #endif
			event_reader_next(pl_sensor->mInputReader);
        }
        
    }
    #if SENSORS_DEBUG
    LOGI("pl_sensor_read_events numEventReceived=%d",numEventReceived);
    #endif
    return numEventReceived;
}

PLSensor* pl_sensor_new()
{
    PLSensor* sensor = malloc(sizeof(PLSensor));
    if (sensor==NULL)
        return NULL;
    memset(sensor,0,sizeof(PLSensor));
    
    sensor_init(&sensor->base);

    
    memset(sensor->mPendingEvents, 0, sizeof(sensor->mPendingEvents));

    sensor->mPendingEvents[LightSensor].sensor = ID_L;//SENSOR_TYPE_LIGHT;
    sensor->mPendingEvents[ProximitySensor].sensor = ID_P;//SENSOR_TYPE_PROXIMITY;
	sensor->mInputReader = event_reader_create(8);

    

    sensor->base.open_dev = pl_sensor_open_dev;
    sensor->base.close = pl_sensor_close;
    sensor->base.delete = pl_sensor_delete;
    sensor->base.read_event = pl_sensor_read_event;

    sensor->base.set_delay = NULL;
    sensor->base.enable = pl_sensor_enable;

    return sensor;
    
}

