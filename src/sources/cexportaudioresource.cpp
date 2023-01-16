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
#include "cexportaudioresource.h"

#define AUDIOBUFFERSIZE 1152 * sizeof(float)

CExportAudioResource::CExportAudioResource(CRadioController *radioController)
:   WStreamResource(),
    _wait_for_data(true)
{
    _radioController = radioController;
    _header_sent = false;

    activityCounter = 4000;
}

CExportAudioResource::~CExportAudioResource()
{

}

void CExportAudioResource::prepareStreaming()
{
    if (_prepare_was_called)
    {
        return;
    }
    else
    {
        _radioController->getRadioServer()->log("info", "[CExportAudioResource] Setze Export Flag auf true ...");

        _radioController->setExportFlag(true);

        ringBufferTimer.setInterval([=]{
            observeRingBuffer();
        }, 1);

        _prepare_was_called = true;
    }
}

void CExportAudioResource::handleRequest(const Http::Request &request, Http::Response &response)
{
    Http::ResponseContinuation *continuation = request.continuation();

    activityCounter = 4000;

    if (!_header_sent)
    {
        prepareStreaming();

        this_thread::sleep_for(chrono::seconds(1));

        if (_radioController->getExportEncoderContext())
        {
            response.setMimeType(_radioController->getExportEncoderContext()->getMimeType());
        }
        else
        {
            response.setMimeType("audio/mpeg");
        }
        
        response.addHeader("Server", "Waverider");
        response.addHeader("Transfer-Encoding", "chunked");

        _radioController->getRadioServer()->log("info", "[CExportAudioResource] Let's go ...");
        
        _header_sent = true;
    }

    int size = 0;
    //size = _radioController->getMP3AudioRingBuffer().GetRingBufferReadAvailable();
    size = _radioController->getCompressedAudioRingBuffer().GetRingBufferReadAvailable();
    //cerr << size << " Bytes available in MP3 RingBuffer ..." << endl;

    if (size > 0)
    {
        //int mp3size = 417 * sizeof(float) * 2;
        //char audioCopyBuffer[AUDIOBUFFERSIZE];
        char* audioCopyBuffer = new char[size];
        //size = _radioController->getMP3AudioRingBuffer().getDataFromBuffer(audioCopyBuffer, size);
        size = _radioController->getCompressedAudioRingBuffer().getDataFromBuffer(audioCopyBuffer, size);
        
        /* std::ostringstream out_size;
        out_size << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << size; */
        
        //cerr << "Transfer " << size << " Bytes." << endl;

        //response.out().write(out_size.str().c_str(), sizeof(out_size.str().c_str()));
        response.out().write(audioCopyBuffer, size);
            
        delete[] audioCopyBuffer;
    }
    
    while (_wait_for_data) 
    {
        //cerr << "Wait for Data ..." << activityCounter << endl;

        if (activityCounter == 0)
        {
            break;
        }
        //_radioController->getExportEncoderContext()->createPaddingFrames();
        this_thread::sleep_for(chrono::milliseconds(48));
    } 

    if (activityCounter > 0)
    {
        response.createContinuation();
        response.continuation();
    }
}

void CExportAudioResource::observeRingBuffer() 
{
    //cerr << "Ringbuffer Data?" << activityCounter << endl;
    //int32_t Bytes = _radioController->getMP3AudioRingBuffer().GetRingBufferReadAvailable();
    int32_t Bytes = _radioController->getCompressedAudioRingBuffer().GetRingBufferReadAvailable();

    activityCounter--;
    if (activityCounter <= 0)
    {
        stopStreaming();
    }

    if (Bytes > 0) 
    {
        _wait_for_data = false;
        _start_com = true;
        //ringBufferTimer.stopTimer();
    }  
    else
    {
        _wait_for_data = true;  
    }
}

void CExportAudioResource::stopStreaming()
{
    _radioController->getRadioServer()->log("info", "[CExportAudioResource] ... stopping export stream.");
    
    _radioController->setExportFlag(false);

    _prepare_was_called = false;

    _header_sent = false;

    ringBufferTimer.stopTimer();
}