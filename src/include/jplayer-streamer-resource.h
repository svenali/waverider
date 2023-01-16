/*
 *    Copyright (C) 2021
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
#ifndef _CJPLAYER_STREAMER_RESOURCE_H_
#define _CJPLAYER_STREAMER_RESOURCE_H_

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
extern "C" {
    #include "wavfile.h"
}

using namespace Wt;
using namespace std;

class CRadioController; // Forward Declaration

class CJPlayerStreamerResource : public WStreamResource
{
    public:
        CJPlayerStreamerResource(CRadioController *radioController);
        
        virtual void handleRequest(const Http::Request &request, Http::Response &response);

        void setSampleRate(int as) { audioSampleRate = as; }
        void sendHeader(bool h) { send_header = h; }
        bool isAudioReady() { return start_com; }
        bool isStreaming() { return _activity; }

        void prepareStreaming();
        void prepareWebStreaming();
        void stopStreaming();
        void sendAudioHeaderAgain();    // If record session is still open...
        
        void setChannel(uint32_t serviceId, string serviceName, string channelID);
        void setWebChannel(string serviceName, string url);
        uint32_t getPlayingServiceID() { return _radioStation.serviceId; }
        string getPlayingServiceName() { return _radioStation.serviceName; }
        string getPlayingChannelID() { return _radioStation.channelID; }

    private:
        CRadioController *_radioController;
        int audioSampleRate;

        void observeRingBuffer();
        
        bool _wait_for_data;
        bool start_com = false;
        bool send_header = true;
        char audioCopyBuffer[2 * 32768];
        int _audioCounter;
        bool _activity;

        int64_t pointerToBuffer = 0;
        int64_t bufferedPackages = 0;
        
        struct RADIO_STATION {
            uint32_t serviceId;
            string serviceName;
            string channelID;
            string url;
        };

        RADIO_STATION _radioStation;

        CSimpleTimer ringBufferTimer;
        FILE* fd = nullptr;
};

#endif //_CJPLAYER_STREAMER_RESOURCE_H_