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
 */
#include "cmp3audiocompression.h"

CMP3AudioCompression::CMP3AudioCompression(CRadioController* r)
    :   CAudioCompression(r),
        _streamBuffer(32 * 32768)
{
#if defined(HAVE_FFMPEG)
    _CodecContext = NULL;
    _Codec = NULL;
    _FormatContext = NULL;
    _AVFifo = NULL;
#endif
}

CMP3AudioCompression::CMP3AudioCompression(CRadioController* r, string filename)
    :   CAudioCompression(r, filename),
        _streamBuffer(32 * 32768)
{
#if defined(HAVE_FFMPEG)
    _CodecContext = NULL;
    _Codec = NULL;
    _AVFifo = NULL;
    _FormatContext = NULL;
#endif
}

CMP3AudioCompression::~CMP3AudioCompression()
{
#if defined(HAVE_FFMPEG)
    //avcodec_close(_CodecContext);
    //av_free(_CodecContext);
#endif
}

string CMP3AudioCompression::getMimeType() 
{ 
    return "audio/mpeg"; 
}

void CMP3AudioCompression::start_compression(bool threading)
{
    if (threading)
    {
        _radioController->getRadioServer()->log("info", "[CMP3AudioCompression] Start Thread ...");
        if (_running)
        {
            return;
        }

        _running = true;
        _initcall = true;

        _mainThread = std::thread(&CMP3AudioCompression::encodeAudio, this);
    }
    else
    {
        init_thread_context();
    }
}

void CMP3AudioCompression::stop_compression()
{
    _running = false;

#if defined(HAVE_FFMPEG)
    if (_AVFifo)
    {
        av_audio_fifo_free(_AVFifo);
    }
    if (_CodecContext)
    {
        avcodec_free_context(&_CodecContext);
    }
    if (_recordToFile)
    {
        avio_closep(&_FormatContext->pb);
    }
    else
    {
        av_free(_externBufferConf.pointerToIndividualBuffer);
    }
    avformat_free_context(_FormatContext);
#endif
}

int writeMP3ToRingBuffer(void* opaque, uint8_t* buf, int buf_size)
{
    TMP3BufferConf *buffer_conv = reinterpret_cast<TMP3BufferConf*>(opaque);

    buffer_conv->exportBuffer->putDataIntoBuffer(buf, buf_size);

    return buf_size;
}

void CMP3AudioCompression::init_format_context()
{
    _radioController->getRadioServer()->log("info", "[CMP3AudioCompression] Prepare the FormatContext ...");

#if defined(HAVE_FFMPEG)
    AVIOContext *output_io_context = NULL;
    int output_buffer_size = 32 * 32768;
    uint8_t *output_buffer = (uint8_t*) av_malloc(output_buffer_size);

    AVStream *stream               = NULL;
    int error;
 
    if (_recordToFile)
    {
        /* Open the output file to write to it. */
        if ((error = avio_open(&output_io_context, _Filename.c_str(), AVIO_FLAG_WRITE)) < 0) 
        {
            _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not open output file!");
            exit(error);
        }
    }
    else
    {
        _externBufferConf.exportBuffer = &_radioController->getCompressedAudioRingBuffer();
        _externBufferConf.pointerToIndividualBuffer = output_buffer;
        
        output_io_context = avio_alloc_context(output_buffer, output_buffer_size, 1, reinterpret_cast<void*>(&_externBufferConf), nullptr, &writeMP3ToRingBuffer, nullptr);
    }
    
    /* Create a new format context for the output container format. */
    if (!(_FormatContext = avformat_alloc_context()))    
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not allocate output format context.");
        
        exit(AVERROR(ENOMEM));
    }
 
    /* Associate the output file (pointer) with the container format context. */
    _FormatContext->pb = output_io_context;
    _FormatContext->flags = AVFMT_FLAG_CUSTOM_IO;
    //cerr << "Buffer: " << _FormatContext->pb->buffer << endl;
    //exit(0);
 
    /* Guess the desired container format based on the file extension. */
    const char* virtual_filename = "output.mp3";
    if (_recordToFile)
    {
        virtual_filename = _Filename.c_str(); 
    }
    
    if (!(_FormatContext->oformat = av_guess_format(NULL, virtual_filename, NULL))) 
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not find output file format.");
        
        exit(0);
    }
 
    if (!(_FormatContext->url = av_strdup(virtual_filename))) 
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not allocate url.");
        
        exit(AVERROR(ENOMEM));
    }    

    if (!(stream = avformat_new_stream(_FormatContext, _CodecContext->codec))) 
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not create new stream.");
        
        exit(AVERROR(ENOMEM));
    }

    stream->time_base.den = audioSampleRate;
    stream->time_base.num = 1;

    error = avcodec_parameters_from_context(stream->codecpar, _CodecContext);
    if (error < 0) 
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not initialize stream parameters.");
    }

    _radioController->getRadioServer()->log("info", "[CMP3AudioCompression] FormatContext created ...");
#endif
}

void CMP3AudioCompression::init_codec_context()
{
#if defined(HAVE_FFMPEG)
    /* find the AAC encoder */
    _Codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
    if (!_Codec) 
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Codec not found!");
        return;
    }

    _CodecContext = avcodec_alloc_context3(_Codec);
    _CodecContext->bit_rate = 320000;                   // Good CD Quality
    _CodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;     // Input from ALL Decoders is always FMT_FLTP
    _CodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
    _CodecContext->channels       = 2;                  // Stereo
    _CodecContext->sample_rate    = audioSampleRate;
    //_CodecContext->sample_rate    = 44100;
    //_CodecContext->thread_count   = 1;
    //av_opt_set_int(_CodecContext, "compression_level", 0, 0);

    /* open it */
    if (avcodec_open2(_CodecContext, _Codec, NULL) < 0) 
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not open codec!");
        
        return;
    }
#endif
}

void CMP3AudioCompression::write_file_header()
{
#if defined(HAVE_FFMPEG)
    int error;
    if ((error = avformat_write_header(_FormatContext, NULL)) < 0) 
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not write output file header!");
        
        exit(error);
    }
#endif
}

void CMP3AudioCompression::transcode_buffer(uint8_t* uint8_buf, int16_t* buf, int size16)
{
    for (int i = 0; i < size16; i++)
    {
        uint8_buf[i*2] = (uint8_t) (0x00FF & buf[i]);
        uint8_buf[i*2 + 1] = (uint8_t) (buf[i] >> 8);  
    }
}

void CMP3AudioCompression::buffer_format_test(uint8_t* uint8_buf, int16_t* buf, int size16)
{
    for (int i = 0; i < 4096; i++)
    {
        cout << "uint8_t Byte " << i << ":" << (int) uint8_buf[i*2] << ";" << (int) uint8_buf[i*2 + 1] << endl;
        cout << "int16_t Byte " << i << ":" << (int) buf[i] << ";" << endl;
    }
}

uint8_t** CMP3AudioCompression::init_conv_buffer(int channels, int buffer_size)
{
    uint8_t** conv_data;
    conv_data = new uint8_t*[channels];
    for (int i=0; i<2; i++)
    {
        conv_data[i] = new uint8_t[buffer_size];
    }

    return conv_data;
}

void CMP3AudioCompression::free_conv_buffer(uint8_t** buffer, int channels)
{
    for (int i = 0; i < channels; i++)
    {
        delete[] buffer[i];
    }

    delete[] buffer;
}

#if defined(HAVE_FFMPEG)
SwrContext* CMP3AudioCompression::init_swr_context()
{
    SwrContext *swr = swr_alloc_set_opts(NULL,
            _CodecContext->channel_layout,          // output
            _CodecContext->sample_fmt,              // output
            audioSampleRate,                        // output
            AV_CH_LAYOUT_STEREO,                    // input
            AV_SAMPLE_FMT_S16,                      // input
            audioSampleRate,                        // input
            0,
            NULL);
    int swrInit = swr_init(swr);
    if (swrInit < 0)
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] SWR init error swrInit!");
         
        return NULL;
    }

    return swr;
}
#endif

void CMP3AudioCompression::reset_conv_buffer(uint8_t** buffer, int buffer_size, int channels)
{
    for (int i = 0; i < channels; i++)
    {
        memset(buffer[i], '\0', buffer_size);
    }
}

void CMP3AudioCompression::init_thread_context()
{
#if defined(HAVE_FFMPEG)
    _radioController->getRadioServer()->log("info", "[CMP3AudioCompression] Start encoding PCM Audio to MP3 for export ...");
    
    if (audioSampleRate == 0)
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] AudioSampleRate not set.");
        
        return;
    }
    
    if (!_Codec)
    {
        init_codec_context();
    }

    if (!_FormatContext)
    {
        init_format_context();
    }

    if (!_AVFifo)
    {
        _AVFifo = av_audio_fifo_alloc(
            _CodecContext->sample_fmt,
            2, // Channels have to be initialized as 2 not 3 like with channel_layout!
            1);
    }  

    int ret = 0;
    
    _swr = init_swr_context();
    if (!_swr)
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] SWR could not be initialized!");
        
        return;
    }

    _buffer16_size = FRAME_SIZE * 2;
    _buffer_size = _buffer16_size * 2 * 2;

    // 16 Bit Samples (size for 16 Bits 2 Byte)
    _data16_buffer = (int16_t*) av_malloc(_buffer16_size);
    _conv_data = init_conv_buffer(2, _buffer_size);

    write_file_header();    // WITHOUT THIS av_write_frame CRAHSES!!!
#endif
}

void CMP3AudioCompression::encodeAudio()
{
#if defined(HAVE_FFMPEG)
    init_thread_context();

    int ret = 0;

    while (_running)
    {
        if (_initcall)
        {
            _radioController->getRadioServer()->log("info", "[CMP3AudioCompression] Init Call ... Wait 3 seconds ...");
            
            this_thread::sleep_for(chrono::seconds(3));
            
            _initcall = false;
        }

        int size = _streamBuffer.GetRingBufferReadAvailable();
        
        if (size >= _buffer16_size)
        {
            uint8_t** conv_data = init_conv_buffer(2, _buffer_size);

            AVFrame *input_frame = av_frame_alloc();
            if (!input_frame) 
            {
                cerr << "Could not allocate audio frame!" << endl;
                return;
            }

            input_frame->nb_samples     = FRAME_SIZE;
            input_frame->format         = AV_SAMPLE_FMT_S16;
            input_frame->channel_layout = AV_CH_LAYOUT_STEREO;

            int data_size = _streamBuffer.getDataFromBuffer(_data16_buffer, _buffer16_size);
            
            int error = avcodec_fill_audio_frame(
                    input_frame,
                    2,
                    AV_SAMPLE_FMT_S16,
                    (uint8_t*)_data16_buffer,
                    _buffer_size, 
                    1);

            if (error < 0)
            {
                _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Something went wrong while initializing an input_frame.");
                continue;
            }
                
            ret = swr_convert(
                _swr, 
                conv_data, 
                input_frame->nb_samples, 
                (const uint8_t**) input_frame->data, 
                input_frame->nb_samples);

            if (ret > 0)
            {
                av_frame_free(&input_frame);
                
                if ((error = av_audio_fifo_realloc(
                    _AVFifo, 
                    av_audio_fifo_size(_AVFifo) + FRAME_SIZE)) < 0) 
                {
                    _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not reallocate FIFO.");
                    
                    return;
                }

                /* Store the new samples in the FIFO buffer. */
                if (av_audio_fifo_write(_AVFifo, (void **)conv_data, FRAME_SIZE) < FRAME_SIZE) 
                {
                    _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not write data to FIFO.");
                    
                    return;
                }
                
                if (av_audio_fifo_size(_AVFifo) > _CodecContext->frame_size)
                {
                    encodeFrame();
                    //av_frame_free(&input_frame);
                    free_conv_buffer(conv_data, 2);
                }
            }
        }
        else 
        {
            //int size = _streamBuffer.GetRingBufferReadAvailable();
            //cerr << "[CMP3AudioCompression] " << size << " Bytes only. Wait for INPUT to encode ..." << endl;
            this_thread::sleep_for(chrono::milliseconds(24));    
        }
    }

    swr_free(&_swr);
    av_free(_data16_buffer);
#endif
}

void CMP3AudioCompression::directFeed(int16_t* data, int len)
{
#if defined(HAVE_FFMPEG)
    int ret = 0;

    _buffer16_size = len;
    _buffer_size = _buffer16_size * 2 * 2;
    int frame_size = len / 2;

    uint8_t** conv_data = init_conv_buffer(2, _buffer_size);
    
    AVFrame *input_frame = av_frame_alloc();
    if (!input_frame) 
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not allocate audio frame!");
        
        return;
    }

    input_frame->nb_samples     = frame_size;
    input_frame->format         = AV_SAMPLE_FMT_S16;
    input_frame->channel_layout = AV_CH_LAYOUT_STEREO;

    int error = avcodec_fill_audio_frame(
        input_frame,
        2,
        AV_SAMPLE_FMT_S16,
        (uint8_t*)data,
        _buffer_size, 
        1);

    if (error < 0)
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Something went wrong while initializing an input_frame.");
    }
                
    ret = swr_convert(
        _swr, 
        conv_data, 
        input_frame->nb_samples, 
        (const uint8_t**) input_frame->data, 
        input_frame->nb_samples);

    if (ret > 0)
    {
        av_frame_free(&input_frame);
                
        if ((error = av_audio_fifo_realloc(
            _AVFifo, 
            av_audio_fifo_size(_AVFifo) + frame_size)) < 0) 
        {
            _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not reallocate FIFO.");
            
            return;
        }

        /* Store the new samples in the FIFO buffer. */
        if (av_audio_fifo_write(_AVFifo, (void **)conv_data, frame_size) < frame_size) 
        {
            _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not write data to FIFO.");
            
            return;
        }
                
        if (av_audio_fifo_size(_AVFifo) > _CodecContext->frame_size)
        {
            encodeFrame();
            free_conv_buffer(conv_data, 2);
        }
    }
#endif
}

void CMP3AudioCompression::encodeFrame()
{
#if defined(HAVE_FFMPEG)
    int error = 0;

    /* Temporary storage of the output samples of the frame written to the file. */
    AVFrame *output_frame = av_frame_alloc();
    /* Create a new frame to store the audio samples. */
    if (!output_frame) 
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not allocate output frame.");
        
        return;
    }
    /* Use the maximum number of possible samples per frame.
     * If there is less than the maximum possible frame size in the FIFO
     * buffer use this number. Otherwise, use the maximum possible frame size. */
    const int frame_size = FFMIN(av_audio_fifo_size(_AVFifo), _CodecContext->frame_size);
    output_frame->nb_samples = frame_size;
    output_frame->format = _CodecContext->sample_fmt;
    output_frame->sample_rate = _CodecContext->sample_rate;
    output_frame->channel_layout = _CodecContext->channel_layout;
    output_frame->channels = 2;
    // output_frame->ch_layout.nb_channels = 2; // ab ffmpeg 5.1!

    /* Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified. */
    error = av_frame_get_buffer(output_frame, 0);
    if (error < 0)
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not allocate output frame samples.");
        
        av_frame_free(&output_frame);
        return;
    }
    error = av_frame_make_writable(output_frame);

    /* packet for holding encoded output */
    AVPacket *output_packet = av_packet_alloc();
    if (!output_packet) 
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not allocate the packet!");
        
        return;
    }

    /* Read as many samples from the FIFO buffer as required to fill the frame.
     * The samples are stored in the frame temporarily. */
    int ret = av_audio_fifo_read(_AVFifo, (void **)output_frame->data, frame_size);
    if (ret < frame_size) 
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not read data from FIFO.");
        
        av_frame_free(&output_frame);
        return;
    }
    
    output_frame->pts = _pts;
    _pts += output_frame->nb_samples;

    ret = avcodec_send_frame(_CodecContext, output_frame);
    if (ret < 0) 
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Error sending the frame to the encoder!");
        return;
    }
        
    /* read all the available output packets (in general there may be any
     * number of them */
    ret = avcodec_receive_packet(_CodecContext, output_packet);

    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Error encoding audio frame ... need more DATA ...");
        
        return;
    }
    else if (ret < 0) 
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Error encoding audio frame!");
        
        return;
    }
                
    //cerr << "Write compressed Data to the CompressedAudioRingBuffer" << endl;            
    
    //_radioController->getCompressedAudioRingBuffer().putDataIntoBuffer(output_packet->data, output_packet->size);

    if (av_write_frame(_FormatContext, output_packet) < 0)
    {
        _radioController->getRadioServer()->log("error", "[CMP3AudioCompression] Could not write compressed MP3 packets to file.");
        
        exit(0);
    }

    //cerr << "Wrote " << output_packet->size << " Bytes to CompressedAudioRingBuffer" << endl; 
                
    av_packet_free(&output_packet);
#endif
}

void CMP3AudioCompression::createPaddingFrames()
{
#if defined(HAVE_FFMPEG)
    uint8_t* audio_data = (uint8_t*) av_malloc(_buffer_size);
    int samples = _buffer16_size / 2;

    av_samples_set_silence(&audio_data, 0, samples, 2, AV_SAMPLE_FMT_S16);

    directFeed((int16_t*)audio_data, _buffer16_size);
#endif
}