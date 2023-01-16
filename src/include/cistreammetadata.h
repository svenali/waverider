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
 * 
 * PS: Bestes Containerformat für komprimiertes Streamen! 
 * https://de.wikipedia.org/wiki/Ogg
 * 
 * Deshalb ist OGG auch standardmaessig als AVFormatContext eingestellt für den Weiterexport!
 */
#ifndef _CISTREAMMETADATA_H_
#define _CISTREAMMETADATA_H_

#include "ringbuffer.h"
#include "wradio-controller.h"

#if defined(HAVE_FFMPEG)
extern "C" 
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/dict.h>
    #include <libavformat/avio.h>
    #include <libavutil/fifo.h>
    #include <libavutil/audio_fifo.h>
    #include <libavutil/opt.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/samplefmt.h>
    #include <libswresample/swresample.h>
    #include "wavfile.h"
}
#endif

using namespace std;
using namespace Wt;

static int readFromRingBuffer(void* opaque, uint8_t* buf, int buf_size);
static int64_t seekInStreamBuffer(void *opaque, int64_t offset, int whence);

struct TStreamingBuffer 
{
    uint8_t* buf;
    size_t b_size;
};

class CIStreamMetaData
{
    public:
        CIStreamMetaData(CRadioController* r);
        ~CIStreamMetaData();
        
        void start_analysing();
        void stop_analysing();
        void copyToAnalyseBuffer(uint8_t* data, int len);
        
    private:
        RingBuffer<uint8_t> _metaDataBuffer;
        bool _running = false;
        std::thread _mainThread;
        CRadioController *_radioController;
        TStreamingBuffer _streamingBuffer;
        void init_format_context();
        void analyseThread();
        FILE* _fp;                          // only for testing
        
#if defined(HAVE_FFMPEG)
        AVFormatContext *_FormatContext;
#endif
};

#endif