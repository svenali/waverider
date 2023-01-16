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
#ifndef _CINTERNETDEVICE_H_
#define _CINTERNETDEVICE_H_

#include <Wt/Http/Client.h>
#include <Wt/Http/Message.h>
#include <Wt/WObject.h>
#include <Wt/WServer.h>
#include <Wt/WIOService.h>

#include <thread>
#include <vector>

#include "cinternet-audio.h"
#include "wradio-controller.h"

using namespace Wt;
using namespace std;

class CInternetAudio;

class CInternetDevice : public WObject
{
    public:
        CInternetDevice(ProgrammeHandlerInterface& phi);
        ~CInternetDevice();

        void setInternetChannel(string url);
        int getAudioSamplerate();

        void stop();

    private:
        AsioWrapper::asio::io_context newService;
        Http::Client *_client;
        string _URL;
        thread threadHandle;
        unique_ptr<CInternetAudio> _internetAudio;
        ProgrammeHandlerInterface &_myProgrammeHandler;
        string _AudioContentType;
        bool _readBodyLink = false;
        bool _loadNewURL = false;
        int _metadatabytes;
        int _readedBytes;           // for metadata chunks
        string _ICYMetaData;
        int _restMetaDataBytes;
        
        void run();
        void handleHttpResponse(AsioWrapper::error_code err, const Http::Message& response);
        void handleHeaderResponse(const Http::Message& response);
        void bodyData(string data);
};

#endif