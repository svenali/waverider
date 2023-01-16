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
#if defined(HAVE_ALSA)

#include "calsaoutput.h"
#include <iostream>

#define PCM_DEVICE "default"
#define AUDIOBUFFERSIZE 32768

CAlsaOutput::CAlsaOutput(int chans, unsigned int rate, string outputDevice, CRadioController *r):    
    _Channels(chans),
    _audioBuffer(1024 * AUDIOBUFFERSIZE)
{
    _radioController = r;
    _outputDevice = outputDevice;
    _audioSampleRate = rate;
    //_audioSampleRate = 48000;

    //int err = snd_pcm_open(&_PCM_Handle, _outputDevice.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    int err = snd_pcm_open(&_PCM_Handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) 
    {
        _radioController->getRadioServer()->log("error", "[CALSAOutput] ERROR: Can't open " + string(PCM_DEVICE) + " PCM device. " + string(snd_strerror(err)));
    }

    snd_pcm_hw_params_alloca(&_params);
    snd_pcm_hw_params_any(_PCM_Handle, _params);

    if ((err = snd_pcm_hw_params_set_access(
                    _PCM_Handle, _params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
        _radioController->getRadioServer()->log("error", "[CALSAOutput] ERROR: Can't set interleaved mode. " + string(snd_strerror(err)));

    if ((err = snd_pcm_hw_params_set_format(
                    _PCM_Handle, _params, SND_PCM_FORMAT_S16_LE)) < 0)
        _radioController->getRadioServer()->log("error", "[CALSAOutput] ERROR: Can't set format. " + string(snd_strerror(err)));

    if ((err = snd_pcm_hw_params_set_channels(_PCM_Handle, _params, _Channels)) < 0)
        _radioController->getRadioServer()->log("error", "[CALSAOutput] ERROR: Can't set channels number. " + string(snd_strerror(err)));

    if ((err = snd_pcm_hw_params_set_rate_near(_PCM_Handle, _params, &_audioSampleRate, 0)) < 0)
        _radioController->getRadioServer()->log("error", "[CALSAOutput] ERROR: Can't set rate. " + string(snd_strerror(err)));

    if ((err = snd_pcm_hw_params(_PCM_Handle, _params)) < 0)
        _radioController->getRadioServer()->log("error", "[CALSAOutput] ERROR: Can't set hardware parameters. " + string(snd_strerror(err)));

    _radioController->getRadioServer()->log("info", "[CALSAOutput] PCM name: " + string(snd_pcm_name(_PCM_Handle)));
    _radioController->getRadioServer()->log("info", "[CALSAOutput] PCM state: " + to_string(snd_pcm_state(_PCM_Handle)));
    _radioController->getRadioServer()->log("info", "[CALSAOutput] PCM rate: " + to_string(_audioSampleRate));
    
    snd_pcm_hw_params_get_period_size(_params, &_period_size, 0);
    //_period_size *= 2;
    _radioController->getRadioServer()->log("info", "[CALSAOutput] PCM frame size: " + to_string(_period_size));
    _radioController->getRadioServer()->log("info", "[CALSAOutput] PCM channels: " + to_string(_Channels));
    
    /* if (snd_pcm_hw_params_set_buffer_size(_PCM_Handle, _params, _period_size * 100) < 0)
    {
        fprintf(stderr, "Error setting buffersize.\n");    
    }*/

    snd_pcm_sw_params_t *swparams;
    snd_pcm_sw_params_alloca(&swparams);
    /* get the current swparams */
    err = snd_pcm_sw_params_current(_PCM_Handle, swparams);
    if (err < 0) 
    {
        _radioController->getRadioServer()->log("info", "[CALSAOutput] Unable to determine current swparams for playback: " + string(snd_strerror(err)));
    }
    err = snd_pcm_sw_params_set_start_threshold(
            _PCM_Handle, swparams, (8192 / _period_size) * _period_size);

    if (err < 0) 
    {
        _radioController->getRadioServer()->log("info", "[CALSAOutput] Unable to set start threshold mode for playback: " + string(snd_strerror(err)));
    }

    if ((err = snd_pcm_sw_params(_PCM_Handle, swparams)) < 0) 
    {
        _radioController->getRadioServer()->log("info", "[CALSAOutput] Setting of swparams failed: " + string(snd_strerror(err)));
    }

    if ((err = snd_pcm_prepare(_PCM_Handle)) < 0) 
    {
        _radioController->getRadioServer()->log("info", "[CALSAOutput] Cannot prepare audio interface for use: " + string(snd_strerror(err)));
    }

    /* err = snd_pcm_nonblock(_PCM_Handle, 0); 
    if (err < 0)
    {
        cout << "Nonblock mode not working" << endl;
        exit(0);
    }*/

    snd_pcm_hw_params_get_buffer_size(_params, &_alsa_frame_buffer_size);
    
    //cout << "ALSA Buffer Size " << _alsa_frame_buffer_size << endl;
    //cout << "FrameSize: " << _audioSampleRate * 2 * _Channels / 100 << endl; 

    //_catch_frames = _audioSampleRate * 2 * _Channels / 100 * 2;
    _catch_frames = 2304 * 2;

    _isRunning = false;
}

CAlsaOutput::~CAlsaOutput() {
    //snd_pcm_drain(_PCM_Handle);
    //snd_pcm_close(_PCM_Handle);
}

void CAlsaOutput::closeDevice()
{
    snd_pcm_drain(_PCM_Handle);
    snd_pcm_close(_PCM_Handle);
}

void CAlsaOutput::playPCM(std::vector<int16_t>&& pcm)
{
    _audioBuffer.putDataIntoBuffer(pcm.data(), static_cast<int32_t>(pcm.size()));

    if (!_isRunning)
    {
        _isRunning = true;
        _playingThread = std::thread(&CAlsaOutput::play, this);
    }
}

/* Original playPCM */
void CAlsaOutput::playPCMDirect(vector<int16_t>&& pcm)
{
    if (pcm.empty())
        return;

    const int16_t *data = pcm.data();

    const size_t num_frames = pcm.size() / _Channels;
    size_t remaining = num_frames;

    while (_PCM_Handle and remaining > 0) {
        size_t frames_to_send = (remaining < _period_size) ? remaining : _period_size;

        snd_pcm_sframes_t ret = snd_pcm_writei(_PCM_Handle, data, frames_to_send);

        if (ret == -EPIPE) {
            snd_pcm_prepare(_PCM_Handle);
            //fprintf(stderr, "XRUN\n");
            this_thread::sleep_for(chrono::milliseconds(20));
            break;
        }
        else if (ret < 0) 
        {
            _radioController->getRadioServer()->log("info", "[CALSAOutput] ERROR: Can't write to PCM device: " + string(snd_strerror(ret)));
            break;
        }
        else {
            size_t samples_read = ret * _Channels;
            remaining -= ret;
            data += samples_read;
        }
    }
}

void CAlsaOutput::play()
{
    this_thread::sleep_for(chrono::seconds(5));

    while (_isRunning)
    {
        int64_t size = _audioBuffer.GetRingBufferReadAvailable();
        
        if (size > 0)
        {
            int64_t total = 0;
            int16_t c_data[_alsa_frame_buffer_size];
            total = _audioBuffer.getDataFromBuffer(c_data, _alsa_frame_buffer_size);
            //int16_t c_data[_catch_frames * 2];
            //total = _audioBuffer.getDataFromBuffer(c_data, _catch_frames * 2);

            const int16_t *data = c_data;

            //const int16_t *data = concrete_data;
            //const size_t num_frames = size / _Channels;
            const size_t num_frames = total / _Channels;
            size_t remaining = num_frames;

            while (_PCM_Handle and remaining > 0) {
                size_t frames_to_send = (remaining < _period_size) ? remaining : _period_size;
                
                snd_pcm_sframes_t ret = snd_pcm_writei(_PCM_Handle, data, frames_to_send);
                //snd_pcm_sframes_t ret = snd_pcm_writei(_PCM_Handle, data, remaining);
                
                /* cout << "Frames to Send: " << frames_to_send << endl;
                cout << "Remaining: " << remaining << endl;
                cout << "Period size: " << _period_size << endl; */

                this_thread::sleep_for(chrono::milliseconds(5));

                if (ret == -EPIPE) 
                {
                    snd_pcm_prepare(_PCM_Handle);
                    //fprintf(stderr, "XRUN\n");
                    this_thread::sleep_for(chrono::milliseconds(20));
                    break;
                }
                else if (ret < 0) 
                {
                    _radioController->getRadioServer()->log("info", "[CALSAOutput] ERROR: Can't write to PCM device. " + string(snd_strerror(ret)));
                    
                    break;
                }
                else 
                {
                    int16_t samples_read = ret * _Channels;
                    remaining -= ret;
                    data += samples_read;
                }
            }
        }
        else
        {
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
}

void CAlsaOutput::stop()
{
    _isRunning = false;
    _playingThread.join();
}

void CAlsaOutput::getSounddeviceList(list<string>* list)
{
    char** hints;
    int    err;
    char** n;
    char*  name;
    char*  desc;
    char*  ioid;

    /* Enumerate sound devices */
    err = snd_device_name_hint(-1, "pcm", (void***)&hints);
    if (err != 0) 
    {
        _radioController->getRadioServer()->log("info", "[CALSAOutput] *** Cannot get device names. ");
        exit(1);
    }

    n = hints;
    while (*n != NULL) {

        name = snd_device_name_get_hint(*n, "NAME");
        desc = snd_device_name_get_hint(*n, "DESC");
        ioid = snd_device_name_get_hint(*n, "IOID");

        /* printf("Name of device: %s\n", name);
        printf("Description of device: %s\n", desc);
        printf("I/O type of device: %s\n", ioid);
        printf("\n"); */

        string new_dev_name(name);
        if (new_dev_name.find("hw") != std::string::npos && new_dev_name.find("hw") == 0) 
        {
            list->push_back(new_dev_name);
        }

        if (name && strcmp("null", name)) free(name);
        if (desc && strcmp("null", desc)) free(desc);
        if (ioid && strcmp("null", ioid)) free(ioid);
        n++;
    }

    //Free hint buffer too
    snd_device_name_free_hint((void**)hints);
}

#endif