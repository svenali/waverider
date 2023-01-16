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

#include "cstreamingserver.h"

CStreamingServer::CStreamingServer(int argc, char *argv[], const std::string &wtConfigurationFile)
:   WServer(argc, argv, wtConfigurationFile)
{
    log("info", "[CStreamingServer] Streaming Server is listening");
}

CStreamingServer::~CStreamingServer()
{
    log("info", "[CStreamingServer] Streaming Server will be killed ...");
}

void CStreamingServer::setInternetChannel(string url)
{
    log("info", "[CStreamingServer] setChannel to URL: " + url);
    
    _URL = url;
    
    streamingTimer.setInterval([=]{
        startChannel();
    }, 30);
}

void CStreamingServer::startChannel()
{
    log("info", "[CStreamingServer] Channel started ... ");
    
    streamingTimer.stopTimer();

    _internetDevice = make_unique<CInternetDevice>(*_radioController);

    _internetDevice->setInternetChannel(_URL);
}

void CStreamingServer::stop()
{
    log("info", "[CStreamingServer] Stopping Streaming Server activity ...");
    
    _internetDevice->stop();
}

void CStreamingServer::setRadioController(CRadioController *rc)
{
    _radioController = rc;
    //_internetDevice = make_unique<CInternetDevice>(*_radioController);

    // Teststream
    //_radioController->setRecordToDir("/Users/sven/records");
    //_radioController->setRecordFromChannel("klassik-radio-internetstream");
    //_radioController->setRecordFlag(true);
}

int CStreamingServer::getAudioSamplerate()
{
    return _internetDevice->getAudioSamplerate();
}

void CStreamingServer::log(string type, string message)
{
    WLogEntry entry = _radioServerLogger.entry(type);
    
    entry << WLogger::timestamp << Wt::WLogger::sep
      << '[' << type << ']' << Wt::WLogger::sep
      << message;
}