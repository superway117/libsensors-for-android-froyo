/*
 * Copyright (C) 2012 Intel Mobile Communications GmbH
 *
 * Copyright (C) 2008 The Android Open Source Project
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


#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <sys/select.h>

#include <linux/input.h>


#include <cutils/atomic.h>
#include <cutils/log.h>
#include "SensorsProxy.h"
#include "SensorsDef.h"

#define __MAX(a,b) ((a)>=(b)?(a):(b))

/*****************************************************************************/

#define MAX_NUM_SENSORS 5

#define SUPPORTED_SENSORS  ((1<<MAX_NUM_SENSORS)-1)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

/*****************************************************************************/

struct sensors_control_context_t {
    struct sensors_control_device_t device; // must be first
    uint32_t active_sensors;
    SensorsProxy* sensor_proxy;
};

struct sensors_data_context_t {
    struct sensors_data_device_t device; // must be first
    SensorsProxy* sensor_proxy;
};

/*
 * The SENSORS Module
*/

static const struct sensor_t sSensorList[] = 
{
#if ENABLE_BOSCH_SENSOR
        { "BMC050 3-axis Accelerometer",
          "Bosh",
          1,
          SENSORS_HANDLE_BASE + ID_A,
          SENSOR_TYPE_ACCELEROMETER,
          4.0f*9.805f,
		      (4.0f*9.805f)/(1<<10),
          (0.03f),
          { }
		    },
        { "BMC050 3-axis Magnetic",
          "Bosh",
          1,
          SENSORS_HANDLE_BASE + ID_M,
          SENSOR_TYPE_MAGNETIC_FIELD,
          1700.0f,
          0.3f,
          (0.5f),
          { }
		    },
        { "BMC050 3-axis Orientation",
          "Bosh",
          1,
          SENSORS_HANDLE_BASE + ID_O,
          SENSOR_TYPE_ORIENTATION,
          360.0f,
          1.0f,
          (0.53f),
          { }
        },
#endif
        { "Avago Light sensor",
          "The Android Open Source Project",
          1,
          SENSORS_HANDLE_BASE + ID_L,
          SENSOR_TYPE_LIGHT,
          360.0f,
          1.0f,
          9.7f,
          { }
        },
        { "Avago Proximity sensor",
          "The Android Open Source Project",
          1,
          SENSORS_HANDLE_BASE + ID_P,
          SENSOR_TYPE_PROXIMITY,
          5.0f,
          1.0f,
          9.7f,
          { }
       },
};

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static int sensors__get_sensors_list(struct sensors_module_t* module,
        struct sensor_t const** list)
{
    UNUSED(module);
    *list = sSensorList;
    return ARRAY_SIZE(sSensorList);
}

static struct hw_module_methods_t sensors_module_methods = {
    .open = open_sensors
};

const struct sensors_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = SENSORS_HARDWARE_MODULE_ID,
        .name = "BMC050 & CM3602 Sensors Module",
        .author = "The Android Open Source Project",
        .methods = &sensors_module_methods,
    },
    .get_sensors_list = sensors__get_sensors_list
};

/*****************************************************************************/

static native_handle_t* control__open_data_source(struct sensors_control_device_t *dev)
{
    struct sensors_control_context_t *ctx = (struct sensors_control_context_t *)dev;
    if(ctx->sensor_proxy == NULL)
        ctx->sensor_proxy = sensors_proxy_get_instance();
    return sensors_proxy_open_all_dev(ctx->sensor_proxy);
}

static int control__activate(struct sensors_control_device_t *dev,int handle, int enabled)
{
    int ret = 0;
    struct sensors_control_context_t *ctx = (struct sensors_control_context_t *)dev;

    if ((handle < SENSORS_HANDLE_BASE) || (handle >= SENSORS_HANDLE_BASE+MAX_NUM_SENSORS))
        return -1;

    ret = sensors_proxy_enable(ctx->sensor_proxy,handle,enabled);
    if(ret==0)
    {
      if(enabled)
        ctx->active_sensors |= 1 << handle;
      else
        ctx->active_sensors &= ~(1 << handle);
    }
    #if SENSORS_DEBUG
    LOGD("control__activate active_sensors=%d",ctx->active_sensors);
    #endif

    return ret;
}

static int control__set_delay(struct sensors_control_device_t *dev, int32_t ms)
{
  struct sensors_control_context_t *ctx = (struct sensors_control_context_t *)dev;
  return sensors_proxy_set_delay(ctx->sensor_proxy,ms);
}

static int control__wake(struct sensors_control_device_t *dev)
{
  struct sensors_control_context_t *ctx = (struct sensors_control_context_t *)dev;
  return sensors_proxy_wake(ctx->sensor_proxy);

}

/*****************************************************************************/

static int data__data_open(struct sensors_data_device_t *dev, native_handle_t* handle)
{
   
  struct sensors_data_context_t *ctx = (struct sensors_data_context_t *)dev;
  if(ctx->sensor_proxy == NULL)
    ctx->sensor_proxy = sensors_proxy_get_instance();
  sensors_proxy_set_fds(ctx->sensor_proxy,handle);
  // Framework will close the handle
  native_handle_delete(handle);
  return 0;
}

static int data__data_close(struct sensors_data_device_t *dev)
{
  struct sensors_data_context_t *ctx = (struct sensors_data_context_t *)dev;
  return sensors_proxy_close_all(ctx->sensor_proxy);
}



static int data__poll(struct sensors_data_device_t *dev, sensors_data_t* values)
{
  struct sensors_data_context_t *ctx = (struct sensors_data_context_t *)dev;
  return sensors_proxy_poll(ctx->sensor_proxy,values);
}

/*****************************************************************************/

static int control__close(struct hw_device_t *dev)
{
    struct sensors_control_context_t* ctx = (struct sensors_control_context_t*)dev;
    if (ctx) 
    {
        sensors_proxy_delete(ctx->sensor_proxy);
        free(ctx);
    }
    return 0;
}

static int data__close(struct hw_device_t *dev)
{
    struct sensors_data_context_t* ctx = (struct sensors_data_context_t*)dev;
    if (ctx) 
    {
        data__data_close((struct sensors_data_device_t*)dev);
        sensors_proxy_delete(ctx->sensor_proxy);
        free(ctx);
    }
    return 0;
}


/** Open a new instance of a sensor device using name */
static int open_sensors(const struct hw_module_t* module, const char* name,struct hw_device_t** device)
{
    int status = -EINVAL;
    if (!strcmp(name, SENSORS_HARDWARE_CONTROL)) 
    {
        struct sensors_control_context_t *dev;
        dev = malloc(sizeof(*dev));
        memset(dev, 0, sizeof(*dev));
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = 0;
        dev->device.common.module = (struct hw_module_t*)module;
        dev->device.common.close = control__close;
        dev->device.open_data_source = control__open_data_source;
        dev->device.activate = control__activate;
        dev->device.set_delay= control__set_delay;
        dev->device.wake = control__wake;

        *device = &dev->device.common;
    } 
    else if (!strcmp(name, SENSORS_HARDWARE_DATA)) 
    {
        struct sensors_data_context_t *dev;
        dev = malloc(sizeof(*dev));
        memset(dev, 0, sizeof(*dev));
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = 0;
        dev->device.common.module = (struct hw_module_t*)module;
        dev->device.common.close = data__close;
        dev->device.data_open = data__data_open;
        dev->device.data_close = data__data_close;
        dev->device.poll = data__poll;
        *device = &dev->device.common;
    }
    return 0;
}
