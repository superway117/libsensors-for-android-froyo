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

 
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>

#include <sys/cdefs.h>
#include <sys/types.h>

#include <linux/input.h>

#include <cutils/log.h>

#include "InputEventReader.h"

/*****************************************************************************/

struct input_event;



InputEventCircularReader* event_reader_create(size_t numEvents)
{
	InputEventCircularReader* reader = malloc(sizeof(InputEventCircularReader));
	if (NULL == reader)
        return NULL;
	reader->mBuffer = malloc(sizeof(struct input_event)*numEvents*2);
	reader->mBufferEnd = reader->mBuffer + numEvents;
	reader->mHead = reader->mBuffer;
	reader->mCurr = reader->mBuffer;
	reader->mFreeSpace = numEvents;
	return reader;
	
}

void event_reader_delete(InputEventCircularReader* reader)
{
	free(reader->mBuffer);
	free(reader);
}


ssize_t event_reader_fill(InputEventCircularReader* reader,int fd)
{
    size_t numEventsRead = 0;
    LOGD("event_reader_fill fd=%d",fd);
    if (reader->mFreeSpace) 
	{
        const ssize_t nread = read(fd, reader->mHead, reader->mFreeSpace * sizeof(struct input_event));
        if (nread<0 || nread % sizeof(struct input_event)) 
		{
            // we got a partial event!!
            return nread<0 ? -errno : -EINVAL;
        }

        numEventsRead = nread / sizeof(struct input_event);
        if (numEventsRead) 
		{
            reader->mHead += numEventsRead;
            reader->mFreeSpace -= numEventsRead;
            if (reader->mHead > reader->mBufferEnd) {
                size_t s = reader->mHead - reader->mBufferEnd;
                memcpy(reader->mBuffer, reader->mBufferEnd, s * sizeof(struct input_event));
                reader->mHead = reader->mBuffer + s;
            }
        }
    }

    return numEventsRead;
}

void event_reader_next(InputEventCircularReader* reader)
{
	reader->mCurr++;
    reader->mFreeSpace++;
    if (reader->mCurr >= reader->mBufferEnd) 
	{
        reader->mCurr = reader->mBuffer;
    }
}


ssize_t event_reader_read(InputEventCircularReader* reader,struct input_event ** events)
{
    ssize_t available = 0;
    *events = reader->mCurr;
    available = (reader->mBufferEnd - reader->mBuffer) - reader->mFreeSpace;
    return available ? 1 : 0;
}