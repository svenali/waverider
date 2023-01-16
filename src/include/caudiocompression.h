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
#ifndef _CAUDIOCOMPRESSION_H_
#define _CAUDIOCOMPRESSION_H_

#include "stdint.h"
#include "ringbuffer.h"

using namespace std;

enum AudioCompression { AAC, ALAC, FLAC, MP2, MP3, VORBIS, WAV };

class CRadioController;
      
class CAudioCompression
{
    public:
        CAudioCompression(CRadioController* r);
        CAudioCompression(CRadioController* r, string filename);
        virtual ~CAudioCompression() {};
        virtual void setSampleRate(unsigned int sr) = 0;
        virtual unsigned int getSampleRate() = 0;
        virtual RingBuffer<uint16_t>& getStreamBuffer() = 0;
        
        virtual void start_compression(bool threading) = 0;
        virtual void stop_compression() = 0;

        virtual void directFeed(int16_t* data, int len) = 0;
        virtual void createPaddingFrames() = 0;

        virtual string getMimeType() = 0;
    
    protected:
        CRadioController* _radioController;
        bool _recordToFile;
        string _Filename;
};

#endif