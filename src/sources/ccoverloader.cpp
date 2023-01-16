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
#include "ccoverloader.h"
#include <boost/algorithm/string.hpp>

CCoverLoader::CCoverLoader(CRadioController *r)
:   WObject(),
    _radioController(r)
{
    _running = true;
    
    _RadioStationFavIconRequestLink = "";
    _TitleRequest = "";

    _state = INITIAL;

    _mainThread = std::thread(&CCoverLoader::run, this);
}  

void CCoverLoader::run()
{
    while(_running)
    {
        this_thread::sleep_for(chrono::seconds(1));
        if (_titleBuffer.size() > 0 && _state == INITIAL)
        {
            _TitleRequest = _titleBuffer.top();
            _titleBuffer.pop();

            _bodyData = "";
            _state = INITIAL;
            // Did we get a StreamTitle?
            // in an iTunes-Style?
            if (itunesFormat(_TitleRequest))
            {
                string Title = "";
                //string Artist = "";
                string Url = "";

                _radioController->getRadioServer()->log("info", "[CCoverLoader] iTunes format identified.");
                this->analyseITunesString(_TitleRequest, Title, _Artist, Url);

                _radioController->getRadioServer()->log("info", "[CCoverLoader] Found: (Title) " + Title + " (Artist) " + _Artist + " (Url) " + Url);
                
                _radioController->onArtistAndTitle(
                    "Artist/Group: " + _Artist +
                    " Title: " + Title);

                if (Url.length() > 0 && Url.find("null") == string::npos)
                {
                    // direct download the mot_pic
                    _TitleRequest = "";
                    _state = COVER_DOWNLOAD;
                    this->init(Url);
                }
                else
                {
                    _TitleRequest = "";
                    _radioController->onNoCoverFound();
                }
                _TitleRequest = "";
            }
            else
            {
                _radioController->getRadioServer()->log("info", "[CCoverLoader] Individual format identified.");
                
                string StreamTitle = getPureStreamTitle(_TitleRequest);
                string Title = getTitle(StreamTitle);
                _Artist = getArtist(StreamTitle);
                _radioController->onArtistAndTitle(
                    "Artist/Group: " + _Artist +
                    " Title: " + Title);
                _TitleRequest = getTitleDataRequest(Title);
                _radioController->getRadioServer()->log("info", "[CCoverLoader] TITLE Request: " + _TitleRequest);
                
                init(_TitleRequest);
                _TitleRequest = ""; 
            }
        }
        /* else
        {
            cerr << "State is " << _state << endl;
            cerr << "Not allowed to search ..." << endl;
        }*/

        if (_RadioStationFavIconRequestLink.length() > 0 && _state == FAVICON)
        {
            _radioController->getRadioServer()->log("info", "[CCoverLoader] No Cover found ... Download favicon.");
            _bodyData = "";
            _state = FAVICON;
            init(_RadioStationFavIconRequestLink);
            _RadioStationFavIconRequestLink = "";
        }
    }
}

void CCoverLoader::init(string url)
{
    _radioController->getRadioServer()->log("info", "[CCoverLoader] Initialize Thread for CCoverLoader ...");
    
    WIOService s;
    s.setThreadCount(1);
    s.start();

    _client = new Http::Client(s);
    
    // without Certification not good, but hey, we talk about internet radio channels
    // and not about a bank ;-)
    _client->setSslCertificateVerificationEnabled(true);

    _client->setTimeout(chrono::seconds{15});
    _client->setMaximumResponseSize(0);
    _client->setFollowRedirect(true);
    _client->headersReceived().connect(bind(&CCoverLoader::handleHeaderResponse, this, placeholders::_1));
    _client->done().connect(bind(&CCoverLoader::handleHttpResponse, this, placeholders::_1,placeholders::_2));
    _client->bodyDataReceived().connect(bind(&CCoverLoader::bodyData, this, placeholders::_1));

    Http::Message::Header app_header("User-Agent","Waverider/0.0.1 (svenali@gmx.de)");
    _v.push_back(app_header);
    Http::Message::Header accept_header("Accept","application/json");
    _v.push_back(accept_header);

    if (_client->get(url, _v)) 
    {
        _radioController->getRadioServer()->log("info", "[CCoverLoader] url " + url + " called.");
    }
    else
    {
        _radioController->getRadioServer()->log("error", "[CCoverLoader] Error while calling the url: " + url);
    }
}

CCoverLoader::~CCoverLoader()
{

}

bool CCoverLoader::itunesFormat(string metadata)
{
    return (
        metadata.find("text") != string::npos || metadata.find("song_spot") != string::npos ||
        metadata.find("MediaBaseId") != string::npos || 
        metadata.find("itunesTrackId") != string::npos ||
        metadata.find("amgTrackId") != string::npos ||
        metadata.find("amgArtistId") != string::npos ||
        metadata.find("amgArtworkURL") != string::npos);
}

string CCoverLoader::getPureStreamTitle(string metadata)
{
    string StreamTitle = "";
    vector<string> strs;
    boost::split(strs, metadata, boost::is_any_of(";"));
    for (size_t i = 0; i < strs.size(); i++)
    {
        vector<string> termin;
        boost::split(termin, strs[i], boost::is_any_of("="));
        for (size_t o = 0; o < termin.size(); o++)
        {
            if (termin[o] == "StreamTitle")
            {
                StreamTitle = termin[o + 1];
                StreamTitle = StreamTitle.substr(1, StreamTitle.length()-2);
                break;
            }
        }

        if (StreamTitle.length() > 0)
        {
            break;
        }
    }

    return StreamTitle;
}

string CCoverLoader::getArtist(string streamtitle)
{
    vector<string> strs;
    boost::split(strs, streamtitle, boost::is_any_of("-"));
    string artist = strs[0];
    boost::trim(artist);

    return artist;
}

string CCoverLoader::getTitle(string streamtitle)
{
    vector<string> strs;
    boost::split(strs, streamtitle, boost::is_any_of("-"));
    if (strs.size() > 1)
    {
        string title = strs[1];
        boost::trim(title);

        return title;
    }

    return streamtitle;
}

string CCoverLoader::getTitleDataRequest(string title)
{
    string t = title;
    boost::replace_all(t, " ", "%20");

    string http_str = "https://musicbrainz.org/ws/2/recording?query=";

    return http_str + "\"" + t + "\"" + "%20AND%20status:official%20ANDprimarytype:album&inc=releases&fmt=json";
}

string CCoverLoader::getReleaseDataRequest(string id)
{
    string http_str = "https://musicbrainz.org/ws/2/release/";

    return http_str + id;
}

string CCoverLoader::getCoverData(string id)
{
    string http_str = "https://coverartarchive.org/release/";

    return http_str + id;
}

void CCoverLoader::searchCoverFor(string metadata)
{
    if (metadata.length() == 0)
    {
        return;
    }

    if (metadata.compare("StreamTitle='';") == 0)
    {
        return;
    }

    //_TitleRequest = metadata;
    _titleBuffer.push(metadata);
    //_TitleRequest = "StreamTitle='ЮРИЙ НИКУЛИН - ПЕСНЯ ПРО ЗАЙЦЕВ';"; // Only for Testing
}

void CCoverLoader::bodyData(string data)
{
    _bodyData += data;
}

void CCoverLoader::handleHeaderResponse(const Http::Message& response)
{
    _radioController->getRadioServer()->log("info", "[CCoverLoader] handleHeaderResponse: receiving content ...");
    
    vector<Http::Message::Header> h = response.headers();
    for (vector<Http::Message::Header>::const_iterator it=h.begin(); it != h.end(); it++)
    {
        Http::Message::Header header = *it;
        _radioController->getRadioServer()->log("info", "[CCoverLoader] handleHeaderResponse: " + header.name());
        _radioController->getRadioServer()->log("info", "[CCoverLoader] handleHeaderResponse: " + header.value());
        //cout << header.name() << " : ";
        //cout << header.value() << endl;
    }
}

string CCoverLoader::findBestMatchingID(map<WString, WString> artists)
{
    map<int, string> levinsteinMap;

    map<WString, WString>::iterator it;
    for (it = artists.begin(); it != artists.end(); ++it)
    {
        WString s = it->second;
        int d = levinstein(s.toUTF8(), _Artist);

        if (d == 0)
        {
            // identical Strings
            return (it->first).toUTF8();
        }
        else
        {
            levinsteinMap[d] = (it->first).toUTF8();
        }
    }

    for (auto const& p : levinsteinMap)
    {
        std::cout << "Smallest Element after Levinstein: " << p.first << ' ' << p.second << '\n';
        return p.second;
    }

    return "";
}

string CCoverLoader::getMusicBrainzReleaseID(string json)
{
    try
    {
        map<WString, WString> idAndArtistMap;

        Json::Object result;
        Json::parse(json, result);

        if (result.get("releases") != nullptr)
        {
            _radioController->getRadioServer()->log("info", "[CCoverLoader] Get Release-ID's");
            vector<Json::Value> array = result.get("recordings");

            if (array.size() > 0)
            {
                for (vector<Json::Value>::iterator it = array.begin() ; it != array.end(); ++it)
                {
                    Json::Object o = (Json::Object) *it;

                    // Artists
                    WString name_artist;
                    vector<Json::Value> artists = o.get("artist-credit");
                    for (vector<Json::Value>::iterator it3 = artists.begin() ; it3 != artists.end(); ++it3)
                    {  
                        Json::Object artist = (Json::Object) *it3;
                        name_artist = artist.get("name");
                        break; // enough
                    }

                    vector<Json::Value> rel_array =  o.get("releases");
                    for (vector<Json::Value>::iterator it2 = rel_array.begin() ; it2 != rel_array.end(); ++it2)
                    {
                        Json::Object r = (Json::Object) *it2;
                        // ID of Release
                        WString id = r.get("id");
                        idAndArtistMap[id] = name_artist;
                        
                        //return id.toUTF8();
                    }
                }
                if (idAndArtistMap.size() > 0)
                    return findBestMatchingID(idAndArtistMap);
                else
                    return "";
            }
        }

        _state = FAILURE;
        return "";
    }
    catch(const Json::TypeException e)
    {
        _radioController->getRadioServer()->log("error", "[CCoverLoader] JSON Error: " + string(e.what()));

        _state = FAILURE;
        return "";
    }
    catch(const Json::ParseError e)
    {
         _radioController->getRadioServer()->log("error", "[CCoverLoader] JSON Error: " + string(e.what()));

        _state = FAILURE;
        return "";
    }
}

string CCoverLoader::getCoverLinkSmallThumbnail(string bodydata)
{
    _radioController->getRadioServer()->log("info", "[CCoverLoader] getCoverLinkSmallThumbnail ...");
    
    try
    {
        // images -> thumbnails
        Json::Object result;
        Json::parse(bodydata, result);

        map<string, Json::Value> r = result;
        for (map<string, Json::Value>::iterator it = r.begin(); it != r.end(); ++it)
        {
            //cerr << "First: " << (*it).first << endl; //" Second: " << (*it).second << endl;

            if ((*it).first == "images")
            {
                vector<Json::Value> thumbs = (*it).second;
                for (vector<Json::Value>::iterator it2 = thumbs.begin() ; it2 != thumbs.end(); ++it2)
                {   
                    try
                    {
                        Json::Object o = *it2;
                        WString link_string = ((Json::Object) o.get("thumbnails")).get("250").toString().orIfNull(WString::Empty);

                        if (link_string.empty())
                        {
                            link_string = ((Json::Object) o.get("thumbnails")).get("small");
                        }

                        cerr << link_string.toUTF8() << endl;

                        return link_string.toUTF8();
                    }
                    catch(const std::exception& e)
                    {
                        _radioController->getRadioServer()->log("error", "[CCoverLoader] getCoverLinkSmallThumbnail Exception: " + string(e.what()));

                        return "";
                    }
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        _radioController->getRadioServer()->log("error", "[CCoverLoader] getCoverLinkSmallThumbnail Exception: " + string(e.what()));

        return "";
    }

    return "";   
}

bool CCoverLoader::hasBodyCoverData(string bodydata)
{
    return (bodydata.find("404 Not Found") == string::npos);
}

mot_file_t CCoverLoader::getImageFromCover(string bodydata)
{
    mot_file_t motfile;
    
    for (int b = 0; b < bodydata.size(); b++)
    {
        motfile.data.push_back(bodydata.data()[b]);
    }

    motfile.content_sub_type = 1;

    return motfile;
}

void CCoverLoader::loadFavIcon(string link)
{
    _RadioStationFavIconRequestLink = link;
}

void CCoverLoader::handleHttpResponse(AsioWrapper::error_code err, const Http::Message& response)
{
    switch (_state)
    {
        case INITIAL:
        {
            string id = getMusicBrainzReleaseID(_bodyData);
            if (id.length() != 0)
            {
                _state = RELEASE;
                string url = getCoverData(id);
                _bodyData = "";
                if (_client->get(url, _v))
                {
                    _radioController->getRadioServer()->log("info", "[CCoverLoader] url: " + url + " successfully called.");
                }
                else
                {
                    _radioController->getRadioServer()->log("info", "[CCoverLoader] Error while calling url: " + url + ".");
                }
            }
            else
            {
                _state = FAILURE;
                _radioController->getRadioServer()->log("info", "[CCoverLoader] No Cover found.");
                _state = FAVICON;
                delete _client;
                _bodyData = "";
                _radioController->onNoCoverFound();
            }
        }
        break;
        case RELEASE:
        {
            if (hasBodyCoverData(_bodyData))
            {
                if (_bodyData.find("See:") != string::npos)
                {
                    string url = _bodyData.substr(5, _bodyData.length() - 6);
                    _bodyData = "";
                    _state = COVER_LINKS;

                    if (_client->get(url, _v))
                    {
                        _radioController->getRadioServer()->log("info", "[CCoverLoader] url: " + url + " successfully called.");
                    }
                    else
                    {
                        _radioController->getRadioServer()->log("error", "[CCoverLoader] Error while calling url: " + url);
                    }
                }
            }
            else
            {
                _state = FAVICON;
                _bodyData = "";
                
                // Reset for next Recognition
                delete _client;
                _radioController->onNoCoverFound();
            }
        }
        break;
        case COVER_LINKS:
        {
            if (hasBodyCoverData(_bodyData))
            {
                string url = getCoverLinkSmallThumbnail(_bodyData);
                _bodyData = "";
                _state = COVER;

                if (url.length() != 0)
                {
                    if (_client->get(url))
                    {
                        _radioController->getRadioServer()->log("info", "[CCoverLoader] url: " + url + " successfully called.");
                    }
                    else
                    {
                        _radioController->getRadioServer()->log("error", "[CCoverLoader] Error while calling: " + url);
                    }
                }
                else
                {
                    _state = FAVICON;
                    // Reset for next Recognition
                    delete _client;
                    _bodyData = "";
                    _radioController->onNoCoverFound();    
                }
            }
            else
            {
                _state = FAVICON;
                // Reset for next Recognition
                delete _client;
                _bodyData = "";
                _radioController->onNoCoverFound();
            }
        }
        break;
        case COVER:
        {
            if (_bodyData.find("See:") != string::npos)
            {
                string url = _bodyData.substr(5, _bodyData.length() - 6);
                _bodyData = "";
                _state = COVER_DOWNLOAD;

                if (_client->get(url, _v))
                {
                    _radioController->getRadioServer()->log("info", "[CCoverLoader] url: " + url + " successfully called.");
                }
                else
                {
                    _radioController->getRadioServer()->log("error", "[CCoverLoader] Error while calling url: " + url);
                }
            }
        }
        break;
        case COVER_DOWNLOAD:
        {
            // Generate motdata Image
            mot_file_t mf = getImageFromCover(_bodyData);
            _bodyData = "";
            delete _client;
            _radioController->onMOT(mf);
            _state = INITIAL;
        }
        break;
        case FAVICON:
        {
            mot_file_t mf = getImageFromCover(_bodyData);
            _bodyData = "";
            delete _client;
            _radioController->onMOT(mf);

            _state = INITIAL;
        }
        break;
        case FAILURE:
        {
            _radioController->getRadioServer()->log("info", "[CCoverLoader] No cover available.");
            delete _client;

            _state = INITIAL;
        }
        break;
    }
}

void CCoverLoader::analyseITunesString(string md, string& title, string& artist, string& url)
{
    //cerr << "Metadata is: " << md << endl;
    vector<string> strs;
    boost::split(strs, md, boost::is_any_of("="));
    for (size_t i = 0; i < strs.size(); i++)
    {
        if (title.length() > 0 && artist.length() > 0 && url.length() > 0)
        {
            break;
        }

        string str = strs[i];
        if (str.find("title") != string::npos)
        {
            i++;
            str = strs[i];
            //cerr << "String for Title: " << str;
            int len = str.find("\"", 1) - str.find("\"");
            title = str.substr(1, len - 1);

            if (str.find("artist") != string::npos)
            {
                i++;
                str = strs[i];
                //cerr << "String for Artist: " << str << endl;
                int len = str.find("\"", 1) - str.find("\"");
                artist = str.substr(1, len - 1);    
            }
        }
        else if (str.find("artist") != string::npos)
        {
            i++;
            str = strs[i];
            //cerr << "String for Artist: " << str << endl;
            int len = str.find("\"", 1) - str.find("\"");
            artist = str.substr(1, len - 1);    
        }
        else if (str.find("amgArtworkURL") != string::npos)
        {
            i++;
            str = strs[i];
            int len = str.find("\"", 1) - str.find("\"");
            url = str.substr(1, len - 1);
        }
    }

    // StreamTitle and artist found?
    /* cerr << "Artist: " << artist.length();
    cerr << " Title: " << title.length() << endl;*/
    if (artist.length() == 0 && title.length() == 0)
    {
        boost::split(strs, md, boost::is_any_of("="));
        for (size_t i = 0; i < strs.size(); i++)
        {
            string str = strs[i];
            if (str.find("StreamTitle") != string::npos)
            {
                i++;
                str = strs[i];
                if (str.find("adContent") == string::npos)
                {
                    int len = str.find("-");
                    artist = str.substr(1, len - 2);
                    i++;
                    str = strs[i];
                    len = str.find("\"", 1) - str.find("\"");
                    title = str.substr(1, len - 1);
                    break;
                }
                else
                {
                    break;
                }
            }
        }
    }  
}