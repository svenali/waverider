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
#ifndef _CCOVERLOADER_H_
#define _CCOVERLOADER_H_

#include <Wt/Http/Client.h>
#include <Wt/Http/Message.h>
#include <Wt/WObject.h>
#include <Wt/WServer.h>
#include <Wt/WIOService.h>
#include <Wt/Json/Parser.h>
#include <Wt/Json/Array.h>

#include <thread>
#include <vector>
#include <stack>

#include "wradio-controller.h"

using namespace Wt;
using namespace std;

class CCoverLoader : public WObject
{
    public:
        CCoverLoader(CRadioController *r);
        ~CCoverLoader();

        void searchCoverFor(string metadata);
        void loadFavIcon(string link);

        string getArtist(string streamtitle);
        string getTitle(string streamtitle);

    private:
        enum States { INITIAL, RELEASE, COVER_LINKS, COVER, COVER_DOWNLOAD, FAILURE, FAVICON };
        States _state;

        AsioWrapper::asio::io_context newService;
        Http::Client *_client;
        string _URL;
        thread threadHandle;
        CRadioController *_radioController;
        string _bodyData;
        string _TitleRequest;
        string _Artist;
        string _RadioStationFavIconRequestLink;
        bool _running = false;
        std::thread _mainThread;
        vector<Http::Message::Header> _v;
        stack<string> _titleBuffer;
        
        void run();
        void init(string url);
        void handleHttpResponse(AsioWrapper::error_code err, const Http::Message& response);
        void handleHeaderResponse(const Http::Message& response);
        void bodyData(string data);

        string findBestMatchingID(map<WString, WString> artists);
        string getPureStreamTitle(string metadata);
        string getMusicBrainzReleaseID(string json);

        string getTitleDataRequest(string title);
        string getReleaseDataRequest(string id);
        string getCoverData(string id);
        bool hasBodyCoverData(string bodydata);
        string getCoverLinkSmallThumbnail(string bodydata);
        mot_file_t getImageFromCover(string bodydata);

        // Metadata Formats
        bool itunesFormat(string metadata);
        void analyseITunesString(string md, string& title, string& artist, string& url);
};

#endif