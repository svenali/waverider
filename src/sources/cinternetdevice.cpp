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
#include "cinternetdevice.h"
#include <boost/algorithm/string.hpp>

CInternetDevice::CInternetDevice(ProgrammeHandlerInterface& phi)
:   WObject(),
    _myProgrammeHandler(phi)
{
    _internetAudio = make_unique<CInternetAudio>(_myProgrammeHandler);

    _AudioContentType = "";

    _metadatabytes = -1;
    _readedBytes = 0;
    _ICYMetaData = "";
    _restMetaDataBytes = 0;

    _client = nullptr;
}

void CInternetDevice::run()
{
    ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("info", "[CInternetDevice] Initialize Thread ...");
    
    WIOService s;
    s.setThreadCount(10);
    s.start();

    _client = new Http::Client(s);
    
    // without Certification not good, but hey, we talk about internet radio channels
    // and not about a bank ;-)
    _client->setSslCertificateVerificationEnabled(true);

    _client->setTimeout(chrono::seconds{15});
    _client->setMaximumResponseSize(0);
    _client->setFollowRedirect(true);
    _client->headersReceived().connect(bind(&CInternetDevice::handleHeaderResponse, this, placeholders::_1));
    _client->done().connect(bind(&CInternetDevice::handleHttpResponse, this, placeholders::_1,placeholders::_2));
    _client->bodyDataReceived().connect(bind(&CInternetDevice::bodyData, this, placeholders::_1));

    // ICY Metadata acception:
    Http::Message::Header icy_header("Icy-MetaData","1");
    vector<Http::Message::Header> v;
    v.push_back(icy_header);
    
    bool retrieveMetadata = ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->getRetrieveMetadata();

    if (retrieveMetadata)
    {
        ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("info", "[CInternetDevice] Retrieve also metadata from this channel.");
        
        if (_client->get(_URL, v))
        {
            ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("info", "[CInternetDevice] url " + _URL + " successfully called.");
        }    
        else 
        {
            ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetDevice] Error calling url " + _URL);
        }
    }
    else
    {
        ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("info", "[CInternetDevice] Not using metadata...");

        if (_client->get(_URL))
        {
            ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("info", "[CInternetDevice] url " + _URL + " successfully called.");
        }    
        else 
        {
            ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetDevice] Error calling url " + _URL);
        }
    }
}

CInternetDevice::~CInternetDevice()
{
}

void CInternetDevice::stop()
{
    ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("notice", "[CInternetDevice] CInternetDevice is stopping ...");
        
    if (_client != nullptr)
    {
        cerr << "Try to abort Internetconnection ..." << endl;
        _client->abort();
    }    
    
    ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("notice", "[CInternetDevice] CInternetDevice is stopped.");

    _internetAudio->stop();

    _AudioContentType = "";
}

void CInternetDevice::bodyData(string data)
{
    if (_readBodyLink)
    {
        size_t found = data.find("http");
        if (found!=std::string::npos)
        {
            _loadNewURL = true;
            _URL = data;
        }
        found = data.find("https");
        if (found!=std::string::npos)
        {
            _loadNewURL = true;
            _URL = data;
        }
        
        _readBodyLink = false;
    }
    
    if (_metadatabytes > 0)
    {
        int s = data.size();

        if (_restMetaDataBytes > 0)
        {
            int gap = s - _restMetaDataBytes;

            if (gap < 0)
            {
                _restMetaDataBytes = abs(gap);
                _ICYMetaData.append(data.data());
                return;
            }
            else
            {
                _readedBytes = gap; 
                char md[_restMetaDataBytes];
                strncpy(md, data.data(), _restMetaDataBytes);
                _ICYMetaData.append(md);
                
                _internetAudio->getBuffer().putDataIntoBuffer(&data.data()[_restMetaDataBytes], gap);
                
                _restMetaDataBytes = 0;

                boost::trim(_ICYMetaData);
                _myProgrammeHandler.onMetaData(_ICYMetaData);
                _ICYMetaData = "";
                //exit(0);
                return;
            }
        }

        if ((_readedBytes + s) - _metadatabytes > 0) // ???
        {
            //cerr << "IT IS TIME TO READ THE METADATA CHUNK ..." << endl;
            //cerr << _readedBytes << " Bytes readed and data.size() is " << s << endl; 
            int offset = abs(_metadatabytes - _readedBytes);
            //int offset = 0;
            uint8_t byte = data.data()[offset];
            int metadata_length = byte * 16;
            int offset_cut_in = offset + 1;
            int offset_cut_out = offset_cut_in + metadata_length;
            int gap = offset_cut_out - s;
            
            if (metadata_length != 0)
            {
                char md[metadata_length];
                if (gap < 0)    // Streaming Data is inside data object
                {
                    strncpy(md, &data.data()[offset + 1], metadata_length);
                    _ICYMetaData.append(md);
                    
                    _readedBytes = abs(gap);

                    boost::trim(_ICYMetaData);
                    _myProgrammeHandler.onMetaData(_ICYMetaData);
                    _ICYMetaData = "";
                    
                    _internetAudio->getBuffer().putDataIntoBuffer(data.data(), offset);
                    _internetAudio->getBuffer().putDataIntoBuffer(&data.data()[s + gap], abs(gap));
                }
                else
                {
                    strncpy(md, &data.data()[offset + 1], s - offset);
                    _ICYMetaData.append(md);
                    
                    _restMetaDataBytes = gap;

                    _internetAudio->getBuffer().putDataIntoBuffer(data.data(), offset);
                }    
                
                return;
            }
            else
            {
                // No MetaData between Audio Packets
                // at offset is shown 0 Bytes Data
                _internetAudio->getBuffer().putDataIntoBuffer(data.data(), offset);
                _internetAudio->getBuffer().putDataIntoBuffer(
                    &data.data()[offset + 1], 
                    s - offset - 1);
                
                _readedBytes = s - offset - 1;

                return;
            }    
        }

        _readedBytes += s;
    }

    if (_AudioContentType.length() != 0)
        _internetAudio->getBuffer().putDataIntoBuffer(data.data(), data.size());
}

int CInternetDevice::getAudioSamplerate()
{
    return _internetAudio->getAudioSamplerate();
}

void CInternetDevice::handleHeaderResponse(const Http::Message& response)
{
    vector<Http::Message::Header> h = response.headers();
    for (vector<Http::Message::Header>::const_iterator it=h.begin(); it != h.end(); it++)
    {
        Http::Message::Header header = *it;
        //cout << header.name() << " : ";
        //cout << header.value() << endl;
        
        if (header.name() == "icy-metaint")
        {
            sscanf(header.value().c_str(), "%d", &_metadatabytes);
        }

        if (header.name() == "content-type" || header.name() =="Content-Type")
        {
            //cout << header.value() << endl;
            
            if (header.value() == "audio/mpeg")
            {
                _AudioContentType = "MP3";
                _internetAudio->setCodec(_AudioContentType);
                _client->get(_URL);
            }
            else if (header.value() == "audio/aac")
            {
                _AudioContentType = "AAC";
                _internetAudio->setCodec(_AudioContentType);
                _client->get(_URL);
            }
            else if (header.value() == "audio/x-mpegurl")
            {
                // read the in body included link
                _readBodyLink = true;
            }
        }
    }
}

void CInternetDevice::handleHttpResponse(AsioWrapper::error_code err, const Http::Message& response)
{
    if (_loadNewURL)
    {
        _loadNewURL = false;
        _readBodyLink = false;
        _client->get(_URL);
    }

    if (err)
    {
        ((CRadioController*) &_myProgrammeHandler)->getRadioServer()->log("error", "[CInternetDevice] Error: " + err.message());
    }
}

void CInternetDevice::setInternetChannel(string url)
{
    _URL = url;
    ((CRadioController*) &_myProgrammeHandler)->setChannelSwitch(true);
    run();
}