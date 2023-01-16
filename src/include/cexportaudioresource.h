/*
 *    Copyright (C) 2022
 *    Dr. Sven Alisch (svenali@gmx.de)
 *
 *    This file is part of the waverider.
 *    waverider is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    waverider is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with waverider; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _CEXPORT_AUDIO_RESOURCE_H_
#define _CEXPORT_AUDIO_RESOURCE_H_

#include <Wt/WResource.h>
#include <Wt/WStreamResource.h>
#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

#include <iostream>
#include <thread>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "ringbuffer.h"
#include "wradio-controller.h"
#include "simple-timer.h"

using namespace Wt;
using namespace std;

class CRadioController; // Forward Declaration

class CExportAudioResource : public WStreamResource
{
    public:
        CExportAudioResource(CRadioController *radioController);
        ~CExportAudioResource();

        virtual void handleRequest(const Http::Request &request, Http::Response &response);
        void prepareStreaming();

        bool isAudioReady() { return _start_com; }

    private:
        void observeRingBuffer();
        void stopStreaming();

        CRadioController *_radioController;
        int audioSampleRate;
        bool _wait_for_data;
        bool _start_com = false;
        bool _prepare_was_called = false;
        bool _header_sent = false;
        int activityCounter;

        CSimpleTimer ringBufferTimer;
};

#endif // _CEXPORT_AUDIO_RESOURCE_H_