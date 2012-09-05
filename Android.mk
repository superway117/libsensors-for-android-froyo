# Copyright (C) 2012 Intel Mobile Communications GmbH.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


LOCAL_PATH := $(call my-dir)

ifneq ($(TARGET_SIMULATOR),true)
ifeq ($(TARGET_PRODUCT),xmm2231ff1_lily)

# HAL module implemenation, not prelinked, and stored in
# hw/<SENSORS_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)

LOCAL_MODULE := sensors.$(TARGET_BOARD_PLATFORM)



LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

#LOCAL_MODULE_TAGS := debug


LOCAL_CFLAGS += -Wall\
		-D LOG_TAG=\"LibSensors\"\
		-D SENSORS_DEBUG=0

LOCAL_SRC_FILES := 						\
				sensors.c 				\
				SensorsProxy.c 			\
				InputEventReader.c	\
				SensorBase.c			\
				PLSensor.c

ifeq ($(ENABLE_BOSCH_SENSOR),true)
LOCAL_CFLAGS += -D ENABLE_BOSCH_SENSOR=1
LOCAL_SRC_FILES += BmcSensor.c
else
LOCAL_CFLAGS += -D ENABLE_BOSCH_SENSOR=0
endif


				
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)


endif # !TARGET_PRODUCT
endif # !TARGET_SIMULATOR
