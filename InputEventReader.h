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

#ifndef ANDROID_INPUT_EVENT_READER1_H
#define ANDROID_INPUT_EVENT_READER1_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>
//#include <linux/input.h>

/*****************************************************************************/

struct input_event;

typedef struct InputEventCircularReader
{
	struct input_event* mBuffer;
    struct input_event* mBufferEnd;
    struct input_event* mHead;
    struct input_event* mCurr;
    ssize_t mFreeSpace;
}InputEventCircularReader;


extern InputEventCircularReader* event_reader_create(size_t numEvents);

extern void event_reader_delete(InputEventCircularReader* reader);

extern ssize_t event_reader_fill(InputEventCircularReader* reader,int fd);

extern void event_reader_next(InputEventCircularReader* reader);

extern ssize_t event_reader_read(InputEventCircularReader* reader,struct input_event ** events);
/*****************************************************************************/

#endif  // ANDROID_INPUT_EVENT_READER_H
