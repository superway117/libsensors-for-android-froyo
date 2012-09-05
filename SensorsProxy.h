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

#ifndef ANDROID_SENSORS_PROXY_H
#define ANDROID_SENSORS_PROXY_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <linux/input.h>

#include <linux/ioctl.h>  /* For IOCTL macros */

#include <hardware/hardware.h>
#include <hardware/sensors.h>

#include <cutils/native_handle.h>

__BEGIN_DECLS

typedef struct _SensorsProxy SensorsProxy;

int sensors_proxy_delete(SensorsProxy* proxy);

SensorsProxy* sensors_proxy_get_instance();

extern int sensors_proxy_set_delay(SensorsProxy* proxy,int32_t ms);

extern int sensors_proxy_enable(SensorsProxy* proxy,int32_t handle, int enable);

extern int sensors_proxy_wake(SensorsProxy* proxy);

extern int sensors_proxy_poll(SensorsProxy* proxy,sensors_data_t* data);

extern int sensors_proxy_open_dev(SensorsProxy* proxy,int32_t handle);

extern native_handle_t* sensors_proxy_open_all_dev(SensorsProxy* proxy);

extern int sensors_proxy_close(SensorsProxy* proxy,int32_t handle);

extern int sensors_proxy_close_all(SensorsProxy* proxy);

extern native_handle_t* sensors_proxy_set_fds(SensorsProxy* proxy,native_handle_t* handle);

__END_DECLS

#endif  // ANDROID_SENSORS_PROXY_H
