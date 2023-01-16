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
#include "cnoaudiocompression.h"

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

CNoAudioCompression::CNoAudioCompression(CRadioController* r)
    :   CAudioCompression(r),
        _streamBuffer(32 * 32768)
{
    _fd = nullptr;
}

CNoAudioCompression::CNoAudioCompression(CRadioController* r, string filename)
    :   CAudioCompression(r, filename),
        _streamBuffer(32 * 32768)
{
    _fd = nullptr;
}

CNoAudioCompression::~CNoAudioCompression()
{

}

string CNoAudioCompression::getMimeType() 
{ 
    return "audio/wav";
}

void CNoAudioCompression::start_compression(bool threading)
{
    if (_recordToFile)
    {
        _fd = wavfile_open(_Filename.c_str(), audioSampleRate, 2);

        if (not _fd)
        {
            _radioController->getRadioServer()->log("error", "[CNoAudioCompression] Could not write to file: " + _Filename);
        }
    }
    else
    {
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

        uint8_t h[sizeof(header)];
        memcpy(h, &header, sizeof(header));

        _radioController->getCompressedAudioRingBuffer().putDataIntoBuffer(&h, sizeof(header));
    }
}

void CNoAudioCompression::stop_compression()
{
    if (_fd)
    {
        wavfile_close(_fd);
        _fd = nullptr;
    }
}

void CNoAudioCompression::directFeed(int16_t* data, int len)
{
    if (_recordToFile)
    {
        wavfile_write(_fd, data, len);
    }
    else
    {
        _radioController->getCompressedAudioRingBuffer().putDataIntoBuffer(data, len);
    }
}