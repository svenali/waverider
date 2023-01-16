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
#ifndef _CMP3AUDIOCOMPRESSION_H_
#define _CMP3AUDIOCOMPRESSION_H_

#include "ringbuffer.h"
#include "caudiocompression.h"
#include "wradio-controller.h"

#if defined(HAVE_FFMPEG)
extern "C" 
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libavformat/avio.h>
    #include <libavutil/fifo.h>
    #include <libavutil/audio_fifo.h>
    #include <libavutil/opt.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/samplefmt.h>
    #include <libswresample/swresample.h>
    #include "wavfile.h"
}

#define INBUF_SIZE 4096
#define FRAME_SIZE 2048
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

#endif

using namespace std;
using namespace Wt;

struct TMP3BufferConf {
        RingBuffer<uint8_t> *exportBuffer;
        uint8_t *pointerToIndividualBuffer;
};

static int writeMP3ToRingBuffer(void* opaque, uint8_t* buf, int buf_size);

class CMP3AudioCompression : public CAudioCompression
{
    public:
        CMP3AudioCompression(CRadioController* r);
        CMP3AudioCompression(CRadioController* r, string filename);
        virtual ~CMP3AudioCompression();
        virtual void setSampleRate(unsigned int sr) { audioSampleRate = sr; };
        virtual unsigned int getSampleRate() { return audioSampleRate; };
        virtual RingBuffer<uint16_t>& getStreamBuffer() { return _streamBuffer; };
        //RingBuffer<uint8_t>& getExportBuffer() { return _exportBuffer; };
        bool dataMP3Encoded() { return _dataMP3Encoded; };
        void dataTransfered() { _dataMP3Encoded = false; };
        
        virtual void start_compression(bool threading);
        virtual void stop_compression();

        virtual void directFeed(int16_t* data, int len);
        virtual void createPaddingFrames();

        virtual string getMimeType();
    
    private:
        RingBuffer<uint16_t> _streamBuffer;
        //RingBuffer<uint8_t> _exportBuffer;
        unsigned int audioSampleRate = 48000;
        bool _running = false;
        bool _initcall = false;
        bool _dataINBuffer = false;
        bool _dataMP3Encoded = false;
        std::thread _mainThread;
        void transcode_buffer(uint8_t* uint8_buf, int16_t* buf, int size16);
        void buffer_format_test(uint8_t* uint8_buf, int16_t* buf, int size16);
        void init_thread_context();
        void init_codec_context();
        void init_format_context();
        void write_file_header();
        uint8_t** init_conv_buffer(int channels, int buffer_size);
        void free_conv_buffer(uint8_t** buffer, int channels);
        void reset_conv_buffer(uint8_t** buffer, int buffer_size, int channels);
#if defined(HAVE_FFMPEG)
        SwrContext* init_swr_context();
        SwrContext* _swr;
        int _buffer16_size;
        int _buffer_size;

        // 16 Bit Samples (size for 16 Bits 2 Byte)
        int16_t* _data16_buffer = NULL;
        uint8_t** _conv_data;
#endif

#if defined(HAVE_FFMPEG)
        const AVCodec *_Codec;
        AVCodecContext *_CodecContext;
        AVFormatContext *_FormatContext;
        AVAudioFifo *_AVFifo;
        int64_t _pts = 0;
#endif

        void encodeAudio();
        void encodeFrame();

        TMP3BufferConf _externBufferConf;
};

#endif
