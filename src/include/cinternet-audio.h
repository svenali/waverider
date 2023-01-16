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
#ifndef _CINTERNETAUDIO_H_
#define _CINTERNETAUDIO_H_

#include "wradio-controller.h"
#include "ringbuffer.h"

#include <mpg123.h>
#include <out123.h>
#include "tools.h"
#include "subchannel_sink.h"

#if defined(HAVE_FFMPEG)
extern "C" 
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avio.h>
    #include <libavutil/opt.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/samplefmt.h>
    #include <libswresample/swresample.h>
    #include "wavfile.h"
}

#define INBUF_SIZE 4096
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

#endif

#define AVCODEC_STANDARD_MP3 -1;

using namespace std;
using namespace Wt;

class CInternetAudio
{
    public:
        CInternetAudio(ProgrammeHandlerInterface& phi);
        ~CInternetAudio();

        RingBuffer<uint8_t>& getBuffer() { return _streamBuffer; };
        int getAudioSamplerate() { return audioSamplerate; };
        void setCodec(string codec) { _CodecString = codec; };

        void stop();

    protected:
        ProgrammeHandlerInterface &_myProgrammeHandler;

    private:
        void run(void);
        bool _running;
        std::thread _mainThread;
        string _CodecString;
#if defined(HAVE_FFMPEG)
        enum AVSampleFormat getSampleFormat(AVCodecContext* cc);
        
        const AVCodec *_Codec;
        AVCodecContext *_CodecContext;
        SwrContext *_swr;
#endif
        RingBuffer<uint8_t> _streamBuffer;
        
        // MP3 Stuff
        void decodeStandardMP3();
        size_t DecodeFrame(uint8_t **data);
        void PutAudio(const uint8_t *data, size_t len);
        void ProcessFormat();
        string audioFormat;
        int audioSamplerate;
        int audioChannels;

        int _channels;
        int _encoding;
        long _rate;
        mpg123_handle *_mh;

#if defined(HAVE_FFMPEG)
        // AAC and OGG and so on stuff
        bool _initNewCodec;
        void decodePacket(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame, SwrContext *swr, uint8_t *pcmAudioBuffer);
        void decodeAudio();
#endif
};

#endif