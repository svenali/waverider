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
#include "coriginalaudiocompression.h"

COriginalAudioCompression::COriginalAudioCompression(CRadioController* r)
    :   CAudioCompression(r),
        _sampleBuffer(1)
{
    _fd = nullptr;
}

COriginalAudioCompression::COriginalAudioCompression(CRadioController* r, string filename)
    :   CAudioCompression(r, filename),
        _sampleBuffer(1)
{
    _fd = nullptr;
}

COriginalAudioCompression::~COriginalAudioCompression()
{

}

string COriginalAudioCompression::getMimeType() 
{ 
    return "audio/wav";
}

void COriginalAudioCompression::start_compression(bool threading)
{
    if (_recordToFile)
    {
        _fd = fopen(_Filename.c_str(), "wb");

        if (not _fd)
        {
            _radioController->getRadioServer()->log("error", "[CNoAudioCompression] Could not write to file: " + _Filename);
        }
    }
}

void COriginalAudioCompression::stop_compression()
{
    if (_fd)
    {
        fclose(_fd);
        _fd = nullptr;
    }
}

void COriginalAudioCompression::directFeed(int16_t* data, int len)
{
    uint8_t* original_data = (uint8_t*) data;   // Because this is calling from onNewCompressedData from the CRadioController

    if (_recordToFile)
    {
        fwrite(original_data, len, 1, _fd);
    }
}