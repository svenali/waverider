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
#include "wwav-streamer-resource.h"

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

CWavStreamerResource::CWavStreamerResource(shared_ptr<CRadioController> radioController)
:   WStreamResource(),
    _wait_for_data(true)
{
    _radioController = radioController;
}

void CWavStreamerResource::prepareStreaming()
{
    activityCounter = 100;
    _activity = false;
    _audioCounter = 20;
    ringBufferTimer.setInterval([=]{
        observeRingBuffer();
    }, 10);
}

void CWavStreamerResource::setChannel(uint32_t serviceId, string serviceName, string channelID)
{
    _radioStation.channelID = channelID;
    _radioStation.serviceId = serviceId;
    _radioStation.serviceName = serviceName;
}

void CWavStreamerResource::handleRequest(const Http::Request &request, Http::Response &response)
{
    Http::ResponseContinuation *continuation = request.continuation();

    activityCounter = 100;

    cout << "Ich glaub es geht schon wieder los ... " << endl;

    // prepare Controller
    if (!start_com)
    {
        _radioController->openDevice();
        _radioController->play(_radioStation.channelID, _radioStation.serviceName, _radioStation.serviceId);

        start_com = true;

        prepareStreaming();

        this_thread::sleep_for(chrono::seconds(3));
    }

    audioSampleRate = _radioController->currentSampleRate();

    while (audioSampleRate == -1) 
    {
        this_thread::sleep_for(chrono::seconds(3));
        audioSampleRate = _radioController->currentSampleRate();
    }
    
    //_activity = true;
    
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

        cout << "HEADER " << endl;

        if (_audioCounter > 0) 
        {
            _audioCounter--;
            cout << "Audio Counter: " << _audioCounter << endl;
        }
        else
        {
            _audioCounter = 20;
            send_header = false;
            //_activity = true;
        }
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

    this_thread::sleep_for(chrono::milliseconds(48));
    
    if (isAudioReady())
    {
        _activity = true;
        cout << "audio ready ..." << endl;
        response.createContinuation();
        response.continuation();
    }
    else
        cout << "audio not ready" << endl;
}

// Helper for char* to base64
string CWavStreamerResource::base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) 
{
    string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) 
    {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) 
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';
    }

    return ret;
}

string CWavStreamerResource::base64_decode(std::string const& encoded_string) 
{
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) 
    {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) 
        {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

void CWavStreamerResource::observeRingBuffer() 
{
    //cout << "ActivityCounter: " << activityCounter << endl;
    int32_t Bytes = _radioController->getAudioRingBuffer().GetRingBufferReadAvailable();

    if (Bytes) 
    {
        _wait_for_data = false;
        start_com = true;

        if (_activity)
        {
            activityCounter--;

            if (activityCounter == 0) 
            {
                stopStreaming();
            }
        }  

        //_audioThreadHandler = thread(&CWavStreamerResource::updateAudioCopyBuffer, this);

        //ringBufferTimer.stopTimer();
    }  
    else
    {
        _wait_for_data = true;  
    }
}

void CWavStreamerResource::updateAudioCopyBuffer()
{
    int64_t total = 0;
    int64_t len = 2*AUDIOBUFFERSIZE;

    while (!_wait_for_data && bufferedPackages < 2 * 32768 * 100) 
    {
        total = _radioController->getAudioRingBuffer().getDataFromBuffer(audioCopyBuffer, len / 2);
        
        if (total != 0) 
        {
            cout << "Wavriderresource play: " << bufferedPackages << " Length." << endl;
            memcpy(&audioStream[bufferedPackages], audioCopyBuffer, total * 2);
            bufferedPackages += total*2;
        }
    
        this_thread::sleep_for(chrono::milliseconds(48));
    }

    fd = wavfile_open("wave.wav", audioSampleRate, 2);
    fwrite(audioStream, bufferedPackages, 1, fd);
    wavfile_close(fd);
}

void CWavStreamerResource::stopStreaming()
{
    start_com = false;
    send_header = true;

    ringBufferTimer.stopTimer();
    
    this_thread::sleep_for(chrono::seconds(3));

    _radioController->stop();

    cout << "CWavStreamerResource: stopped" << endl;
}