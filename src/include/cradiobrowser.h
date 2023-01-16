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
#ifndef _CRADIOBROWSER_H_
#define _CRADIOBROWSER_H_

#include <Wt/Http/Client.h>
#include <Wt/Http/Message.h>
#include <Wt/WObject.h>
#include <Wt/WServer.h>
#include <Wt/WString.h>
#include <Wt/WIOService.h>
#include <Wt/Json/Parser.h>
#include <Wt/Json/Object.h>
#include <Wt/Json/Array.h>
#include <Wt/Json/Value.h>
#include <thread>

#include "radio-server.h"

using namespace Wt;
using namespace std;

namespace Json = Wt::Json;

enum GetRequestToRadioBrowser { None, Countries, ChannelDownload };

class CRadioServer;
class CRadioBrowser : public WObject
{
    public:
        CRadioBrowser(CRadioServer &r, string urlToRadioBrowserServer);
        ~CRadioBrowser();

        void downloadCountries();
        void downloadChannels(string exactCountryCode);

    private:
        Http::Client *_client;
        string _URL;
        string _answer;
        GetRequestToRadioBrowser _GetMode;
        CRadioServer &_radioServer;
        Json::Array _countryArray;
        vector<Json::Value>::iterator _countryIterator;
        int _countedCountry;
        
        void handleHttpResponse(AsioWrapper::error_code err, const Http::Message& response);
        void bodyData(string data);
        void parseCountries();
        void parseChannels();
};

#endif