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

#ifndef ANDROID_BMC_SENSOR_H
#define ANDROID_BMC_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <hardware/sensors.h>

#include "SensorsDef.h"
#include "SensorBase.h"
#include "InputEventReader.h"

/*****************************************************************************/

//struct sensor_data_t;

#define BMC_SENSOR_NUM_MAX 3



#define BMC_DATA_POLL_TIMEOUT 500000

#define BMC_DEVICE_NAME      "/dev/vsns_misc"

enum SENSOR_HANDLE 
{
	SENSOR_HANDLE_START = 0,
	SENSOR_HANDLE_ACCELERATION=1,	/* 1 */
	SENSOR_HANDLE_MAGNETIC_FIELD=2,	/* 2 */
	SENSOR_HANDLE_ORIENTATION=3,	/* 3 */
    //SENSOR_HANDLE_ORIENTATION_RAW=12,  /* 12 */
};

#define SET_SENSOR_ACTIVE				0x01
#define SET_SENSOR_DELAY				0x02

#define FIFO_CMD "/data/local/tmp/fifo_cmd"
#define FIFO_DAT "/data/local/tmp/fifo_dat"

typedef struct {
    union {
        float v[3];
        struct {
            float x;
            float y;
            float z;
        };
        struct {
            float azimuth;
            float pitch;
            float roll;
        };
    };

    int8_t status;
    uint8_t reserved[3];
} sensor_data_vec_t;

/**
 * Union of the various types of sensor data
 * that can be returned.
 */
typedef struct {
    /* must be sizeof(struct sensors_event_t) */
    int32_t version;

    /* sensor identifier */
    int32_t sensor;

    /* sensor type */
    int32_t type;

    /* reserved */
    int32_t reserved0;

    /* time is in nanosecond */
    int64_t timestamp;

    union {
        float           data[16];

        /* acceleration values are in meter per second per second (m/s^2) */
        sensor_data_vec_t   acceleration;

        /* magnetic vector values are in micro-Tesla (uT) */
        sensor_data_vec_t   magnetic;

        /* orientation values are in degrees */
        sensor_data_vec_t   orientation;

        /* gyroscope values are in rad/s */
        sensor_data_vec_t   gyro;

        /* temperature is in degrees centigrade (Celsius) */
        float           temperature;

        /* distance in centimeters */
        float           distance;

        /* light in SI lux units */
        float           light;

        /* pressure in hectopascal (hPa) */
        float           pressure;
    };

    uint32_t        reserved1[4];
} bmc_sensor_data_vec_t;

struct exchange 
{
	int magic;

	union 
     {
		struct 
        {
			short cmd;
			short code;
			short value;

			unsigned char reserved[3];
		} command;

		bmc_sensor_data_vec_t data;

	};
};
struct bmm_cmd 
{
    short magic;
    short cmd;
    short code;
    short value;
};

#define CHANNEL_PKT_MAGIC_CMD (int)'C'
#define CHANNEL_PKT_MAGIC_DAT (int)'D'

enum DEV_AVAILABILITY{
	UNAVAILABLE = 0, 
	AVAILABLE,
	VIRTUAL
};


#define SENSOR_ACCURACY_UNRELIABLE	0
#define SENSOR_ACCURACY_LOW			1
#define SENSOR_ACCURACY_MEDIUM		2
#define SENSOR_ACCURACY_HIGH		3

#define SENSOR_MAGIC_A 'a'
#define SENSOR_MAGIC_D 'd'
#define SENSOR_MAGIC_G 'g'
#define SENSOR_MAGIC_L 'l'
#define SENSOR_MAGIC_M 'm'
#define SENSOR_MAGIC_O 'o'
#define SENSOR_MAGIC_P 'p'
#define SENSOR_MAGIC_R 'r'	/* raw orientation */
#define SENSOR_MAGIC_T 't'	/* raw orientation */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) ((int)(sizeof(arr) / sizeof((arr)[0])))
#endif



#define BMC_DEVICE_NAME     "/dev/vsns_misc"

typedef struct BmcSensor
{
    SensorBase base;   //must be the first one

    //int mEnabled;
    //int32_t mDelays[BMC_SENSOR_NUM_MAX+1];   
    //int data_fd;
    //int cmd_fd;


}BmcSensor;

extern BmcSensor* bmc_sensor_new();
/*****************************************************************************/

#endif  // ANDROID_AKM_SENSOR_H
#endif //  ENABLE_BOSCH_SENSOR