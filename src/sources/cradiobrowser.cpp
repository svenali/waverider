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
#include "cradiobrowser.h"

CRadioBrowser::CRadioBrowser(CRadioServer &r, string urlToRadioBrowserServer)
:   WObject(),
    _radioServer(r)
{
    _URL = urlToRadioBrowserServer;
    _answer = "";

    _client = new Http::Client();
    
    _client->setTimeout(chrono::seconds{15});
    _client->setMaximumResponseSize(0);
    _client->done().connect(bind(&CRadioBrowser::handleHttpResponse, this, placeholders::_1,placeholders::_2));
    _client->bodyDataReceived().connect(bind(&CRadioBrowser::bodyData, this, placeholders::_1));
}

CRadioBrowser::~CRadioBrowser()
{

}

void CRadioBrowser::downloadCountries()
{
    string getRequest = _URL + "json/countrycodes";

    cout << "Download Countries via URL: " << getRequest << endl;

    _GetMode = GetRequestToRadioBrowser::Countries;

    _client->get(getRequest);
}

void CRadioBrowser::downloadChannels(string exactCountryCode)
{
    string getRequest = _URL + "json/stations/bycountrycodeexact/" + exactCountryCode;

    _radioServer.log("info", "[CRadioBrowser] Download Countries via URL: " + getRequest);
    
    _GetMode = GetRequestToRadioBrowser::ChannelDownload;

    _client->get(getRequest);
}

void CRadioBrowser::bodyData(string data)
{
    //cout << "Body Data Received: " << data << endl;
    _answer += data;
}

void CRadioBrowser::handleHttpResponse(AsioWrapper::error_code err, const Http::Message& response)
{
    vector<Http::Message::Header> h = response.headers();
    for (vector<Http::Message::Header>::const_iterator it=h.begin(); it != h.end(); it++)
    {
        Http::Message::Header header = *it;
        cout << header.name() << " : ";
        cout << header.value() << endl;
    }

    if (err)
    {
        cout << "ERROR: " << err.message() << endl;
    }

    // Request finished => Parse answer
    switch (_GetMode)
    {
        case GetRequestToRadioBrowser::Countries:
            parseCountries();
        break;
        case GetRequestToRadioBrowser::ChannelDownload:
            parseChannels();
        break;
    }
}

void CRadioBrowser::parseCountries()
{
    Json::Array result;
    Json::parse(_answer, result);

    cerr << "Size: " << result.size() << endl;
    for(vector<Json::Value>::iterator it = begin(result); it != end(result); ++it) 
    {
        Json::Object o = *it;
        string c = o.get("name");
        
        _radioServer.addCountry(c);
    }

    // Download Channels Country by Country
    _countryArray = result;
    _countryIterator = begin(_countryArray);
    Json::Object firstCountry = *_countryIterator;

    _answer = "";

    _radioServer.tellCountryCount(_countryArray.size());

    _countedCountry = 0;

    downloadChannels(firstCountry.get("name"));
}

void CRadioBrowser::parseChannels()
{
    Json::Array result;
    Json::parse(_answer, result);

    _countedCountry++;
    
    if (_countryArray.size() == _countedCountry)
    {
        _radioServer.scan_internet_finished();
    }
    else
    {
        for(vector<Json::Value>::iterator it = begin(result); it != end(result); ++it) 
        {
            Json::Object o = *it;
            string c = o.get("name");
            
            // create Channel in RadioServer
            string stationuuid = (o.get("stationuuid").isNull()) ? "" : o.get("stationuuid").toString();
            string name = (o.get("name").isNull()) ? "" : o.get("name").toString();
            string url = (o.get("url").isNull()) ? "" : o.get("url").toString();
            string url_resolved = (o.get("url_resolved").isNull()) ? "" : o.get("url_resolved").toString();
            string homepage = (o.get("homepage").isNull()) ? "" : o.get("homepage").toString();
            string favicon = (o.get("favicon").isNull()) ? "" : o.get("favicon").toString();
            string tags = (o.get("tags").isNull()) ? "" : o.get("tags").toString();
            string country = (o.get("country").isNull()) ? "" : o.get("country").toString();
            string countrycode = (o.get("countrycode").isNull()) ? "" : o.get("countrycode").toString();
            string state = (o.get("state").isNull()) ? "" : o.get("state").toString();
            string language = (o.get("language").isNull()) ? "" : o.get("language").toString();
            string languagecode = (o.get("languagecode").isNull()) ? "" : o.get("languagecode").toString();
            int votes = (o.get("votes").isNull()) ? "" : o.get("votes");
            string codec = (o.get("codec").isNull()) ? "" : o.get("codec").toString();
            int bitrate = (o.get("bitrate").isNull()) ? "" : o.get("bitrate");
            _radioServer.addWebChannel(stationuuid, name, url, url_resolved, homepage, favicon, tags, country, countrycode, state, language, languagecode, votes, codec, bitrate);

            _radioServer.tellCountryScan(_countedCountry, country, countrycode);
        }

        if (_countryIterator != end(_countryArray))
        {
            _answer = "";
            _countryIterator++;
            Json::Object nextCountry = *_countryIterator;
            downloadChannels(nextCountry.get("name"));
        }
    }
}