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

#include "cinternet-audio.h"

CInternetAudio::CInternetAudio(ProgrammeHandlerInterface& phi):
    _myProgrammeHandler(phi),
    _streamBuffer(32 * 32768)
{
#if defined(HAVE_FFMPEG)
    _initNewCodec = true;
#endif

    mpg123_init();
    int err;
    _mh = mpg123_new(NULL, &err);

    if (mpg123_open_feed(_mh) == MPG123_OK)
    {
        ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("info", "[CInternetAudio] Library MPG123 - Feed open.");
    }
    else
    {
        ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] Library MPG123 - Could not open feed.");
    }
    
#if defined(HAVE_FFMPEG)
    //avcodec_register_all();   It was deprecated in FFMPEG 4.0, FFMPEG 5.0 do not support this function 

    _CodecContext = NULL;
#endif

    _running = true;
    _mainThread = std::thread(&CInternetAudio::run, this);
}

CInternetAudio::~CInternetAudio()
{
    mpg123_close(_mh);
    mpg123_delete(_mh);
    mpg123_exit();

#if defined(HAVE_FFMPEG)
    avcodec_close(_CodecContext);
    av_free(_CodecContext);
#endif
}

void CInternetAudio::stop()
{
    _running = false;

    ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("info", "[CInternetAudio] Wait until Audio Thread is undead ...");
    _mainThread.join();
    ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("info", "[CInternetAudio] Audio Thread is dead ...");
}

void CInternetAudio::run()
{
    while (_CodecString.length() == 0)
        this_thread::sleep_for(chrono::seconds(1));      
#if defined(HAVE_FFMPEG)
    if (_CodecString == "MP3")
        decodeStandardMP3();
    else
        decodeAudio();
#else
    decodeStandardMP3();
#endif        
}

#if defined(HAVE_FFMPEG)
enum AVSampleFormat CInternetAudio::getSampleFormat(AVCodecContext* cc)
{
    if (cc->sample_fmt == AVSampleFormat::AV_SAMPLE_FMT_U8)
        return AV_SAMPLE_FMT_U8;
    else if (cc->sample_fmt == AVSampleFormat::AV_SAMPLE_FMT_S16)
        return AV_SAMPLE_FMT_S16;
    else if (cc->sample_fmt == AVSampleFormat::AV_SAMPLE_FMT_S32)
        return AV_SAMPLE_FMT_S32;
    else if (cc->sample_fmt == AVSampleFormat::AV_SAMPLE_FMT_FLT)
        return AV_SAMPLE_FMT_FLT;
    else if (cc->sample_fmt == AVSampleFormat::AV_SAMPLE_FMT_DBL)
        return AV_SAMPLE_FMT_DBL;
    else if (cc->sample_fmt == AVSampleFormat::AV_SAMPLE_FMT_U8P)
        return AV_SAMPLE_FMT_U8P;
    else if (cc->sample_fmt == AVSampleFormat::AV_SAMPLE_FMT_S16P)
        return AV_SAMPLE_FMT_S16P;
    else if (cc->sample_fmt == AVSampleFormat::AV_SAMPLE_FMT_S32P)
        return AV_SAMPLE_FMT_S32P;
    else if (cc->sample_fmt == AVSampleFormat::AV_SAMPLE_FMT_FLTP)
        return AV_SAMPLE_FMT_FLTP;
    else if (cc->sample_fmt == AVSampleFormat::AV_SAMPLE_FMT_DBLP)
        return AV_SAMPLE_FMT_DBLP;
    else
        return AV_SAMPLE_FMT_S16;
}

void CInternetAudio::decodePacket(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame, SwrContext *swr, uint8_t *pcmAudioBuffer)
{
    int ret, data_size;

    /* send packets to the decoder */
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0)
    {
        ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] Error while sending packets to the decoder.");
    }

    // >= 0 "=" is important for AAC
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return;
        }
        else if (ret < 0)
        {
            ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] Error while decoding.");
            
            return;
        }
        int data_size = frame->nb_samples*frame->channels;
        audioChannels = frame->channels;
        swr_convert(swr, &pcmAudioBuffer, frame->nb_samples, (const uint8_t**)frame->extended_data, frame->nb_samples);

        audioSamplerate = frame->sample_rate;
                    
        PutAudio(pcmAudioBuffer, data_size * 2);
    }
}

void CInternetAudio::decodeAudio()
{
    ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] Decoding ... AAC ...");

    //audioSamplerate = 44100;
    audioFormat = "";
    AVCodecParserContext *parser;
    if (_initNewCodec)
    {
        _Codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
        if (!_Codec) 
        {
            ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] Codec not found.");
            
            return;
        }

        parser = av_parser_init(AV_CODEC_ID_AAC);
        if (!parser)
        {
            ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] Could not init Parser ...");
            
            exit(1);
        }

        _CodecContext = avcodec_alloc_context3(_Codec);

        if (avcodec_open2(_CodecContext, _Codec, NULL) < 0) 
        {
            ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] Could not open codec ...");

            return;
        }

        _initNewCodec = false;
    }

    this_thread::sleep_for(chrono::microseconds(3));

    SwrContext *swr = NULL;
    if (!swr)
    {
        swr = swr_alloc_set_opts(NULL,
            AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT, // output
            AV_SAMPLE_FMT_S16,                    // output
            44100,                                // output
            AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT, // input
            AV_SAMPLE_FMT_FLTP,                   // input
            44100,                              // input
            0,
            NULL);
        int swrInit = swr_init(swr);
        if (swrInit < 0)
        {
            ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] SWR init error swrInit.");

            exit(1);
        }
    }

    uint8_t *pcmAudioBuffer = (uint8_t*) av_malloc (AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE);
    
    AVPacket *pkt = av_packet_alloc();
    if (!pkt)
        exit(1);
    
    uint8_t data[(AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE) * 20];
    int buffer_size = (AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE) * 20;
    memset(data, 0, buffer_size);
    uint8_t* pData = data;

    AVFrame *decodedFrame = NULL;
    int len, ret;
    int data_size = 0;

    while (_running)
    {
        int32_t size = _streamBuffer.GetRingBufferReadAvailable();

        if (size > 0)
        {
            if (data_size > 0)
            {
                // There is a rest of data in the buffer
                ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] Here is a bunch of data in the buffer ... rescuing ...");
                
                memmove(data, pData, data_size);
                data_size = _streamBuffer.getDataFromBuffer(&data[data_size], buffer_size - data_size);
            }
            else
                data_size = _streamBuffer.getDataFromBuffer(data, buffer_size);
            
            pData = data;

            // save compressed data
            _myProgrammeHandler.onNewCompressedAudio(data, data_size, false);

            //cerr << "Downloaded " << data_size << " Bytes from Host." << endl;

            while (data_size > 0)
            {
                if (!decodedFrame)
                {
                    if (!(decodedFrame = av_frame_alloc()))
                    {
                        ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] Could not allocate AVFrame ...");
                        
                        exit(1);
                    }
                    else
                    {
                        ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] decodedFrame allocated ...");
                    }
                }

                ret = av_parser_parse2(parser, _CodecContext, &pkt->data, &pkt->size, 
                    pData, data_size, 
                    AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

                //cout << "Parser " << ret << " readed." << endl;

                if (ret < 0)
                {
                    ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] Error while parsing ...");
                    
                    exit(1);
                }

                pData       += ret;
                data_size   -= ret;

                if (pkt->size)
                {
                    //cout << "Decoding this packet with size of ";
                    //cout << pkt->size << " Bytes" << endl;

                    decodePacket(_CodecContext, pkt, decodedFrame, swr, pcmAudioBuffer);
                }

                //cout << data_size << " Bytes in Buffer ..." << endl;

                if (data_size <= 0)
                {
                    // More data
                    // data will be rescued during a new buffer fill!
                    break;
                }
            }
        }
        else
        {
            //cerr << "Wait for INPUT (CInternetAudio)" << endl;
            this_thread::sleep_for(chrono::seconds(1));    
        }
    }

    // CleanUp
    av_parser_close(parser);
    av_frame_free(&decodedFrame);
    av_packet_free(&pkt);
}
#endif

void CInternetAudio::decodeStandardMP3()
{
    ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] Decoding Standard MP3 Audio with MPG123.");
    
    while (_running)
    {
        this_thread::sleep_for(chrono::microseconds(3));

        int32_t size = _streamBuffer.GetRingBufferReadAvailable();

        if (size > 0)
        {
            //uint8_t data[size];                   // causes bus error? (26.11.2022)
            uint8_t *data = new uint8_t[size + 1];
            _streamBuffer.getDataFromBuffer(data, size);

            // save compressed data
            _myProgrammeHandler.onNewCompressedAudio(data, size, false);

            int mpg_result = mpg123_feed(_mh, data, size);
            if(mpg_result != MPG123_OK)
            {
                ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] MP2Decoder: error while mpg123_feed: " + string(mpg123_plain_strerror(mpg_result)));
            }

            mpg123_scan(_mh);
            mpg_result = mpg123_meta_check(_mh);
            //cout << "Result: " << mpg_result << endl;
            switch (mpg_result)
            {
                case MPG123_ID3: 
                {
                    mpg123_id3v1 *v1;
                    mpg123_id3v2 *v2;
                    mpg_result = mpg123_id3(_mh, &v1, &v2);
                    ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("info", "[CInternetAudio] MPG123: Metadata found.");
                }
                break;
                case MPG123_ICY:
                {
                    ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("info", "[CInternetAudio] MPG123: ICY-Metadata found ...");
                } 
                break;
            }

            do {
                // go to next frame
                mpg_result = mpg123_framebyframe_next(_mh);
                switch(mpg_result) 
                {
                    case MPG123_NEED_MORE:
                        break;	// loop left below
                    case MPG123_NEW_FORMAT:
                        ProcessFormat();
                        // fall through - as MPG123_NEW_FORMAT implies MPG123_OK
                    case MPG123_OK: 
                    {
                        // forward decoded frame, if applicable
                        uint8_t *frame_data;
                        size_t frame_len = DecodeFrame(&frame_data);
                        if(frame_len)
                        {
                            //cout << "Frame len: " << frame_len << endl;
                            PutAudio(frame_data, frame_len);
                        }
                        break; 
                    }
                    default:
                    {
                        ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetAudio] MP2Decoder: error while mpg123_framebyframe_next: " + string(mpg123_plain_strerror(mpg_result)));
                    }
                }
            } while (mpg_result != MPG123_NEED_MORE);

            delete[] data;
        }
        else
        {
            this_thread::sleep_for(chrono::seconds(1));    
        }
    }
}

size_t CInternetAudio::DecodeFrame(uint8_t **data)
{
    int mpg_result;

    size_t frame_len;
	mpg_result = mpg123_framebyframe_decode(_mh, nullptr, data, &frame_len);
	if(mpg_result != MPG123_OK)
		throw std::runtime_error("MP2Decoder: error while mpg123_framebyframe_decode: " + std::string(mpg123_plain_strerror(mpg_result)));

	return frame_len;
}

void CInternetAudio::PutAudio(const uint8_t *data, size_t len)
{
    // Then len is given in bytes. For stereo it is the double times of mono.
    // But we need two channels even if we have mono.
    // Mono: len = len / 2 * 2 We have len to devide by 2 and for two channels we have multiply by two
    // Stereo: len = len / 2 We just need to devide by 2 because it is stereo
    size_t bufferSize = audioChannels == 2 ? len/2 : len;
    std::vector<int16_t> audio(bufferSize);

    // Convert two uint8 into a int16 sample
    for(size_t i=0; i<len/2; ++i) {
        int16_t sample =  ((int16_t) data[i * 2 + 1] << 8) | ((int16_t) data[i * 2]);

        if (audioChannels == 2) 
        {
            audio[i] = sample;
        }
        else 
        {
        	// upmix to stereo
            audio[i*2] = sample;
            audio[i*2+1] = sample;
        }
    }
    //cerr << "InternetAudio is sending ... " << audio.size() << endl;
    _myProgrammeHandler.onNewAudio(move(audio), audioSamplerate, audioFormat);
}

void CInternetAudio::ProcessFormat() {
	mpg123_frameinfo info;
	int mpg_result = mpg123_info(_mh, &info);
	if(mpg_result != MPG123_OK)
		throw std::runtime_error("MP2Decoder: error while mpg123_info: " + std::string(mpg123_plain_strerror(mpg_result)));

	//scf_crc_len = (info.version == MPG123_1_0 && info.bitrate < (info.mode == MPG123_M_MONO ? 56 : 112)) ? 2 : 4;

	// output format
	std::string version = "unknown";
	switch(info.version) {
	case MPG123_1_0:
		version = "1.0";
		break;
	case MPG123_2_0:
		version = "2.0";
		break;
	case MPG123_2_5:
		version = "2.5";
		break;
	}
	//lsf = info.version != MPG123_1_0;

	std::string layer = "unknown";
	switch(info.layer) {
	case 1:
		layer = "I";
		break;
	case 2:
		layer = "II";
		break;
	case 3:
		layer = "III";
		break;
	}

	std::string mode = "unknown";
	switch(info.mode) {
	case MPG123_M_STEREO:
		mode = "Stereo";
		break;
	case MPG123_M_JOINT:
		mode = "Joint Stereo";
		break;
	case MPG123_M_DUAL:
		mode = "Dual Channel";
		break;
	case MPG123_M_MONO:
		mode = "Mono";
		break;
	}

	AUDIO_SERVICE_FORMAT format;
	format.codec = "MPEG " + version + " Layer " + layer;
	format.samplerate_khz = info.rate / 1000;
	format.mode = mode;
	format.bitrate_kbps = info.bitrate;
	audioFormat = format.GetSummary();

    audioSamplerate = info.rate;
    //audioSamplerate = 48000;
    audioChannels = info.mode != MPG123_M_MONO ? 2 : 1;
}