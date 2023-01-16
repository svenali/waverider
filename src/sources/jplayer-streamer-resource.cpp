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
#include "jplayer-streamer-resource.h"

#define AUDIOBUFFERSIZE 32768

struct wavfile_header {
    char riff_tag[4];
    int riff_length;
    char wave_tag[4];
    char fmt_tag[4];
    int fmt_length;
    short audio_format;
    short num_channels;
    int sample_rate;
    int byte_rate;
    short block_align;
    short bits_per_sample;
    char data_tag[4];
    int data_length;
};

CJPlayerStreamerResource::CJPlayerStreamerResource(CRadioController *radioController)
:   WStreamResource(),
    _wait_for_data(true)
{
    _radioController = radioController;

    _activity = false;
}

void CJPlayerStreamerResource::prepareWebStreaming()
{
    _radioController->getRadioServer()->log("info", "[CJPlayerStreamerResource] Prepare web streaming ...");
    
    _activity = true;
    send_header = true;
    _audioCounter = 2;

    ringBufferTimer.setInterval([=]{
        observeRingBuffer();
    }, 10);
}

void CJPlayerStreamerResource::prepareStreaming()
{
    _radioController->getRadioServer()->log("info", "[CJPlayerStreamerResource] Prepare streaming ...");

    _activity = true;
    send_header = true;
    _audioCounter = 20;

    ringBufferTimer.setInterval([=]{
        observeRingBuffer();
    }, 10);
    
    audioSampleRate = _radioController->currentSampleRate();
}

void CJPlayerStreamerResource::sendAudioHeaderAgain()
{
    send_header = true;
    _audioCounter = 20;
}

void CJPlayerStreamerResource::setChannel(uint32_t serviceId, string serviceName, string channelID)
{
    _radioStation.channelID = channelID;
    _radioStation.serviceId = serviceId;
    _radioStation.serviceName = serviceName;
}

void CJPlayerStreamerResource::setWebChannel(string serviceName, string url)
{
    _radioStation.serviceName = serviceName;
    _radioStation.url = url;
}

/* original für dab+ */
void CJPlayerStreamerResource::handleRequest(const Http::Request &request, Http::Response &response)
{
    Http::ResponseContinuation *continuation = request.continuation();
    
    if (send_header)
    {   
        //start_com = false;
        response.setMimeType("audio/wav");
        response.addHeader("Server", "Waverider");

        wavfile_header header;
        int samples_per_second = audioSampleRate;
        int bits_per_sample = 16;

        memcpy(header.riff_tag,"RIFF",4);
        memcpy(header.wave_tag,"WAVE",4);
        memcpy(header.fmt_tag,"fmt ",4);
        memcpy(header.data_tag,"data",4);

        header.riff_length = 0;
        header.fmt_length = 16;
        header.audio_format = 1;
        header.num_channels = 2;
        header.sample_rate = samples_per_second;
        header.byte_rate = samples_per_second*(bits_per_sample/8)*2;
        header.block_align = 2*bits_per_sample/8;
        header.bits_per_sample = bits_per_sample;
        header.data_length = 0;

        char h[sizeof(header)];
        memcpy(h, &header, sizeof(header));

        response.out().write(h, sizeof(h));

        send_header = false;
    }

    while (_wait_for_data && isAudioReady()) 
    {
        this_thread::sleep_for(chrono::milliseconds(3));
    }
    
    int64_t total = 0;
    int64_t len = 2*AUDIOBUFFERSIZE;
    char audioCopyBuffer[len];
    total = _radioController->getAudioRingBuffer().getDataFromBuffer(audioCopyBuffer, len / 2);
        
    response.out().write(audioCopyBuffer, 2 * total);
    
    if (total == 0) 
    {
        memset(audioCopyBuffer, 0, len);
        total = len / 2;
    }

    if (isAudioReady() && isStreaming())
    {
        response.createContinuation();
        response.continuation();
    }
}

void CJPlayerStreamerResource::observeRingBuffer() 
{
    int32_t Bytes = _radioController->getAudioRingBuffer().GetRingBufferReadAvailable();

    if (Bytes) 
    {
        _wait_for_data = false;
        start_com = true;
    }  
    else
    {
        _wait_for_data = true;  
    }
}

void CJPlayerStreamerResource::stopStreaming()
{
    start_com = false;
    send_header = true;
    _activity = false;

    ringBufferTimer.stopTimer();
    
    _radioController->getRadioServer()->log("info", "[CJPlayerStreamerResource] stopped.");
}