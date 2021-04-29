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
#ifndef _CWAV_STREAMER_RESOURCE_H_
#define _CWAV_STREAMER_RESOURCE_H_

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

static const string base64_chars = 
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+";

class CRadioController; // Forward Declaration

class CWavStreamerResource : public WStreamResource
{
    public:
        CWavStreamerResource(shared_ptr<CRadioController> radioController);
        
        virtual void handleRequest(const Http::Request &request, Http::Response &response);

        void setSampleRate(int as) { audioSampleRate = as; }
        void sendHeader(bool h) { send_header = h; }
        bool isAudioReady() { return start_com; }

        void stopStreaming();
        void prepareStreaming();

        void setChannel(uint32_t serviceId, string serviceName, string channelID);

    private:
        string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
        string base64_decode(std::string const& encoded_string);

        bool isStreaming;
        //RingBuffer<int16_t>& _audiobuffer;
        shared_ptr<CRadioController> _radioController;
        int audioSampleRate;

        static inline bool is_base64(unsigned char c) {
            return (isalnum(c) || (c == '+') || (c == '/'));
        }

        void observeRingBuffer();
        void updateAudioCopyBuffer();
        bool _wait_for_data;
        bool start_com = false;
        bool send_header = true;
        char audioCopyBuffer[2 * 32768];
        int _audioCounter;
        int activityCounter;  // Resource active?
        bool _activity;

        int64_t pointerToBuffer = 0;
        int64_t bufferedPackages = 0;
        char audioStream[2 * 32768 * 100];
        
        thread _audioThreadHandler;

        struct RADIO_STATION {
            uint32_t serviceId;
            string serviceName;
            string channelID;
        };

        RADIO_STATION _radioStation;

        CSimpleTimer ringBufferTimer;
        FILE* fd = nullptr;
};

#endif //_CWAV_STREAMER_RESOURCE_H_