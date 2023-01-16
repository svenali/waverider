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
#ifndef _CALSAOUTPUT_H_
#define _CALSAOUTPUT_H_

#include <cstddef>
#include <thread>
#include <vector>
#include <list>
#include <string>
#include <alsa/asoundlib.h>

#include "ringbuffer.h"
#include "wradio-controller.h"
extern "C" 
{
    #include "wavfile.h"
}

#define PCM_DEVICE "default"

using namespace std;

class CRadioController;

class CAlsaOutput
{
    public:
        CAlsaOutput(int chan, unsigned int rate, string outputDevice, CRadioController *r);
        ~CAlsaOutput();
        CAlsaOutput(const CAlsaOutput& other) = delete;
        CAlsaOutput& operator=(const CAlsaOutput& other) = delete;

        void playPCM(vector<int16_t>&& pcm);
        void playPCMDirect(vector<int16_t>&& pcm);
        void stop();
        void getSounddeviceList(list<string>* list);

        void closeDevice();
        
        bool isRunning() { return _isRunning; };

    private:
        void play();
        CRadioController *_radioController;
        bool _isRunning;
        std::thread _playingThread;
        int _Channels = 2;
        snd_pcm_uframes_t _period_size;
        snd_pcm_t *_PCM_Handle;
        snd_pcm_hw_params_t *_params;
        string _outputDevice;
        unsigned int _audioSampleRate;
        RingBuffer<int16_t> _audioBuffer;

        size_t _alsa_frame_buffer_size; 
        size_t _catch_frames;
};

#endif // _CALSAOUTPUT_H
#endif // if defined