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
 * 
 * 
 *    Attention:    In FFmpeg, audio frame size refers to samples, not bytes. 
 *                  So one audio frame of a 16-bit 2-channel PCM stream will have 
 *                  1024 x 16 x 2 = 32768 bits = 4096 bytes.
 */
#ifndef _CORIGINALAUDIOCOMPRESSION_H_
#define _CORIGINALAUDIOCOMPRESSION_H_

#include "ringbuffer.h"
#include "caudiocompression.h"
#include "wradio-controller.h"

#define INBUF_SIZE 4096
#define FRAME_SIZE 2048
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

using namespace std;
using namespace Wt;

class COriginalAudioCompression : public CAudioCompression
{
    public:
        COriginalAudioCompression(CRadioController* r);
        COriginalAudioCompression(CRadioController* r, string filename);
        virtual ~COriginalAudioCompression();
        virtual void setSampleRate(unsigned int sr) { audioSampleRate = sr; };
        virtual unsigned int getSampleRate() { return audioSampleRate; };
        virtual RingBuffer<uint16_t>& getStreamBuffer() { return _sampleBuffer; };
        virtual void setFilename(string Filename) {};
        
        virtual void start_compression(bool threading);
        virtual void stop_compression();

        virtual void directFeed(int16_t* data, int len);
        virtual void createPaddingFrames() {};

        virtual string getMimeType();
    
    private:
        unsigned int audioSampleRate = 48000;
        RingBuffer<uint16_t> _sampleBuffer; // unused here make it little!

        FILE *_fd = nullptr;
};

#endif