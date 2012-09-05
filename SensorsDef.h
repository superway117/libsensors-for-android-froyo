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

#ifndef ANDROID_SENSORS_DEF_H
#define ANDROID_SENSORS_DEF_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <linux/input.h>

#include <linux/ioctl.h>  /* For IOCTL macros */

#include <hardware/hardware.h>
#include <hardware/sensors.h>

__BEGIN_DECLS

/*****************************************************************************/

//extern int init_nusensors(hw_module_t const* module, hw_device_t** device);

/*****************************************************************************/

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#if ENABLE_BOSCH_SENSOR

#define ID_A  (0)
#define ID_M  (1)
#define ID_O  (2)
#define ID_L  (3)
#define ID_P  (4)

#define MAX_NUM_SENSORS             5
#else 

#define ID_L  (0)
#define ID_P  (1)

#define MAX_NUM_SENSORS             2
#endif



#define EVENT_TYPE_ACCEL_X          ABS_X
#define EVENT_TYPE_ACCEL_Y          ABS_Z
#define EVENT_TYPE_ACCEL_Z          ABS_Y
#define EVENT_TYPE_ACCEL_STATUS     ABS_WHEEL

#define EVENT_TYPE_YAW              ABS_RX
#define EVENT_TYPE_PITCH            ABS_RY
#define EVENT_TYPE_ROLL             ABS_RZ
#define EVENT_TYPE_ORIENT_STATUS    ABS_RUDDER

#define EVENT_TYPE_MAGV_X           ABS_HAT0X
#define EVENT_TYPE_MAGV_Y           ABS_HAT0Y
#define EVENT_TYPE_MAGV_Z           ABS_BRAKE

#define EVENT_TYPE_TEMPERATURE      ABS_THROTTLE
#define EVENT_TYPE_STEP_COUNT       ABS_GAS


#define EVENT_TYPE_PROXIMITY        ABS_DISTANCE
#define EVENT_TYPE_LIGHT            ABS_MISC

// 720 LSG = 1G
#define LSG                         (720.0f)


// conversion of acceleration data to SI units (m/s^2)
#define CONVERT_A                   (GRAVITY_EARTH / LSG)
#define CONVERT_A_X                 (-CONVERT_A)
#define CONVERT_A_Y                 (CONVERT_A)
#define CONVERT_A_Z                 (-CONVERT_A)

// conversion of magnetic data to uT units
#define CONVERT_M                   (1.0f/16.0f)
#define CONVERT_M_X                 (-CONVERT_M)
#define CONVERT_M_Y                 (-CONVERT_M)
#define CONVERT_M_Z                 (CONVERT_M)

#define CONVERT_O                   (1.0f)
#define CONVERT_O_Y                 (CONVERT_O)
#define CONVERT_O_P                 (CONVERT_O)
#define CONVERT_O_R                 (-CONVERT_O)

#define SENSOR_STATE_MASK           (0x7FFF)


//misc device ioctrl type
#define SENSOR_IOCTL_BASE 77
/** The following define the IOCTL command values via the ioctl macros */
#define SENSOR_IOCTL_SET_DELAY			_IOW(SENSOR_IOCTL_BASE, 0, int)
#define SENSOR_IOCTL_GET_DELAY			_IOR(SENSOR_IOCTL_BASE, 1, int)
#define SENSOR_IOCTL_SET_ENABLE			_IOW(SENSOR_IOCTL_BASE, 2, int)   /*To enable accelerometer engine*/
#define SENSOR_IOCTL_GET_ENABLE			_IOR(SENSOR_IOCTL_BASE, 3, int)
#define SENSOR_IOCTL_SET_TILT_ENABLE	_IOW(SENSOR_IOCTL_BASE, 5, int)  /*To enable TILT engine*/
#define SENSOR_IOCTL_SET_B2S_ENABLE		_IOW(SENSOR_IOCTL_BASE, 6, int)
#define SENSOR_IOCTL_SET_WAKE_ENABLE	_IOW(SENSOR_IOCTL_BASE, 7, int)
#define SENSOR_IOCTL_VCMP_SET_ENABLE	_IOW(SENSOR_IOCTL_BASE, 8, int)  /*To enable Compass*/
#define SENSOR_IOCTL_VPS_SET_ENABLE		_IOW(SENSOR_IOCTL_BASE, 9, int)  /* To enable proximity sensor */
#define SENSOR_IOCTL_VALS_SET_ENABLE	_IOW(SENSOR_IOCTL_BASE, 10, int) /* To enable ambient light sensor */
#define SENSOR_IOCTL_VALS_SET_DELAY		_IOW(SENSOR_IOCTL_BASE, 11, int)


#ifndef UNUSED
# define UNUSED(p) ((void)p)
#endif

/*****************************************************************************/

__END_DECLS

#endif  // ANDROID_SENSORS_DEF_H
