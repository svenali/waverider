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
#include "radio-server.h"

CRadioServer::CRadioServer(int argc, char *argv[], const std::string &wtConfigurationFile, CStreamingServer *ss)
:   WServer(argc, argv, wtConfigurationFile)
{
    _refreshSettings = true;

    _channelChange = false;
    _currentPlayingStationType = CurrentPlayingStationType::noservice;

    _streamingServer = ss;
    _radioController = make_unique<CRadioController>(this, _streamingServer);
    
    /* CWavStreamerResource *source = new CWavStreamerResource(_radioController);
    source->setChannel(0x1258, "Klassik R. Movie", "5D");
    this->addResource(source, "/dab/0x1258"); */

    _jplayerStreamer = make_unique<CJPlayerStreamerResource>(_radioController.get());
    this->addResource(_jplayerStreamer.get(), "/dab/jplayer");

    _exportAudioResource = make_unique<CExportAudioResource>(_radioController.get());
    this->addResource(_exportAudioResource.get(), "/streaming");

    _dbConnector = make_unique<dbo::backend::Sqlite3>("waverider.db"); 
    _session.setConnection(std::move(_dbConnector));

    _session.mapClass<CService>("servicetable");
    _session.mapClass<CSettings>("settings");
    _session.mapClass<CDBOCountry>("countries");
    _session.mapClass<CDBOInternetChannel>("internetchannels");
    _session.mapClass<CDBOBouquet>("bouquets");
    _session.mapClass<CDBORecord>("records");

    initDatabase();
    readSettings();
    
    // prepare dummy mot Image
    ifstream input(this->appRoot() + "mot/test-pattern-640.png", std::ios::binary );
    // copies all data into buffer
    vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});
    _motImage = make_unique<WMemoryResource>("image/png");
    _motImage->setData(buffer);
    this->addResource(_motImage.get(), "/dab/mot");
    mot_resource_path = "/dab/mot";

    _radioBrowser = make_unique<CRadioBrowser>(*this, _settings()->_radioBrowserURL);
}

CRadioServer::~CRadioServer()
{
    this->stop();
}

void CRadioServer::initDatabase()
{
    try
    {
        _session.createTables();
    }
    catch(const std::exception& e)
    {
        log("error", "[CRadioServer] " + string(e.what()));
    }   
}

dbo::ptr<CSettings> CRadioServer::_settings()
{
    if (!_refreshSettings)
    {
        return _readedSettings;
    }
    else
    {
        try
        {
            dbo::Transaction transaction{_session};
            _readedSettings = _session.find<CSettings>().where("id = ?").bind("1");
            _refreshSettings = false;

            return _readedSettings;
        }
        catch (dbo::Exception e)
        {
            // No problem
        }
    }
}

void CRadioServer::readSettings()
{
    // Only for creating new Settings
    try
    {
        dbo::Transaction transaction{_session};
        _readedSettings = _session.find<CSettings>().where("id = ?").bind("1");

        // sometimes (newer versions of wt do not throwing an error here, so THROW!)
        if (_readedSettings.get() == NULL)
        {
            throw(dbo::ObjectNotFoundException("settings", "1"));
        }
    }
    catch (dbo::Exception e)
    {
        // Database not exists. Do scan.
        log("warning", "[CRadioServer] Error in DataBase because no settings are defined. Create a new one...");
        dbo::Transaction transaction{_session};

        //auto s = make_unique<CSettings>();
        _readedSettings = _session.add(unique_ptr<CSettings>{new CSettings()});
        _readedSettings.modify()->_ipaddress = "0.0.0.0";
        _readedSettings.modify()->_port = "1234";
        _readedSettings.modify()->_radioDevice = "rtl-sdr";
        _readedSettings.modify()->_recordPath = "undefined";
        _readedSettings.modify()->_radioBrowserURL = "";
        _readedSettings.modify()->_radioBrowserURL = "http://all.api.radio-browser.info/";
        _readedSettings.modify()->_doNotReEncodeIChannels = "true";
        _readedSettings.modify()->_streaming = "false";
        _readedSettings.modify()->_streamingAddress = "/streaming";
        _readedSettings.modify()->_streamingFormat = "FLAC";
        _readedSettings.modify()->_recordFormat = "FLAC";

        _readedSettings.modify()->_metadata = "true";
        _readedSettings.modify()->_saveMetadata = "true";

        _session.flush();
    } 
}

bool CRadioServer::connect(Client *client, const RadioEventCallback& handleEvent, const SettingEventCallback& handleSettingEvent, const BouquetEditorEventCallback& bouquetEventCallback, const RecordFileEventCallback& recordFileEventCallback, const ErrorEventCallback& errorEventCallback)
{
    unique_lock<recursive_mutex> lock(_mutex);

    // this prevents that the same client is logged in twice
    if (_clients.count(client) == 0)
    {
        ClientInfo clientinfo;
        
        clientinfo.sessionID = WApplication::instance()->sessionId();
        clientinfo.eventCallback = handleEvent;
        clientinfo.settingsCallback = handleSettingEvent;
        clientinfo.bouquetCallback = bouquetEventCallback;
        clientinfo.fileCallback = recordFileEventCallback;
        clientinfo.errorCallback = errorEventCallback;

        _clients[client] = clientinfo;

        return true;
    }
    else
    {
        return false;
    }
}

bool CRadioServer::disconnect(Client *client) 
{
    unique_lock<recursive_mutex> lock(_mutex);

    return _clients.erase(client) == 1;
}

void CRadioServer::postRadioEvent(const RadioEvent& radioEvent)
{
    unique_lock<recursive_mutex> lock(_mutex);

    WApplication *app = WApplication::instance();

    for (ClientMap::const_iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        /* 
            Sollte der User direkt mit der App verbunden sein, den callback direkt aufrufen, sonst
            server push.
        */
        if (app && app->sessionId() == it->second.sessionID)
        {
            it->second.eventCallback(radioEvent);
        }
        else
        {
            this->post(it->second.sessionID, bind(it->second.eventCallback, radioEvent));
        }
    }
}

void CRadioServer::postSettingEvent(const SettingsEvent& settingEvent)
{
    unique_lock<recursive_mutex> lock(_mutex);

    WApplication *app = WApplication::instance();

    for (ClientMap::const_iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        /* 
            Sollte der User direkt mit der App verbunden sein, den callback direkt aufrufen, sonst
            server push.
        */
        if (app && app->sessionId() == it->second.sessionID)
        {
            it->second.settingsCallback(settingEvent);
        }
        else
        {
            this->post(it->second.sessionID, bind(it->second.settingsCallback, settingEvent));
        }
    }
}

void CRadioServer::postBouquetEditorEvent(const BouquetEditorEvent& bouquetEditorEvent)
{
    unique_lock<recursive_mutex> lock(_mutex);

    WApplication *app = WApplication::instance();

    for (ClientMap::const_iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        /* 
            Sollte der User direkt mit der App verbunden sein, den callback direkt aufrufen, sonst
            server push.
        */
        if (app && app->sessionId() == it->second.sessionID)
        {
            it->second.bouquetCallback(bouquetEditorEvent);
        }
        else
        {
            this->post(it->second.sessionID, bind(it->second.bouquetCallback, bouquetEditorEvent));
        }
    }
}

void CRadioServer::postRecordFileEvent(const RecordFileEvent& recordFileEvent)
{
    unique_lock<recursive_mutex> lock(_mutex);

    WApplication *app = WApplication::instance();

    for (ClientMap::const_iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        /* 
            Sollte der User direkt mit der App verbunden sein, den callback direkt aufrufen, sonst
            server push.
        */
        if (app && app->sessionId() == it->second.sessionID)
        {
            it->second.fileCallback(recordFileEvent);
        }
        else
        {
            this->post(it->second.sessionID, bind(it->second.fileCallback, recordFileEvent));
        }
    }
}

void CRadioServer::postErrorEvent(const ErrorEvent& errorEvent)
{
    unique_lock<recursive_mutex> lock(_mutex);

    WApplication *app = WApplication::instance();

    for (ClientMap::const_iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        /* 
            Sollte der User direkt mit der App verbunden sein, den callback direkt aufrufen, sonst
            server push.
        */
        if (app && app->sessionId() == it->second.sessionID)
        {
            it->second.errorCallback(errorEvent);
        }
        else
        {
            this->post(it->second.sessionID, bind(it->second.errorCallback, errorEvent));
        }
    }
}

void CRadioServer::scan_dab()
{
    _radioController->openDevice();
    _radioController->startScan();
}

void CRadioServer::scan_internet()
{
    _radioBrowser->downloadCountries();
}

void CRadioServer::scanStop()
{
    log("notice", "CRadioServer::scanStop() was called ...");
    // Save Services in DB
    // ...
    //_session.mapClass<CService>("servicetable");
    try
    {
        _session.createTables();
    }
    catch (Dbo::Exception e)
    {
        // Nothing to do
    }

    /* dbo::Transaction transaction{_session};

    // Does any channels in the db exists?
    typedef dbo::collection< dbo::ptr<CService> > Channels;
    Channels channels = _session.find<CService>().orderBy("id");
    if (channels.size() > 0)
    {
        // delete existing channels
        for(Channels::const_iterator it=channels.begin(); it != channels.end(); ++it)
        {
            dbo::ptr<CService> s = *it;
            s.remove();
        }  
    }

    for_each(_foundedChannels.begin(), _foundedChannels.end(), [this](CService* s) 
    { 
        auto service = make_unique<CService>();
        service->_serviceID = s->_serviceID;
        service->_label = s->_label;
        service->_currentChannel = s->_currentChannel;
        // Type
        service->_type = "dab+";

        cout << "ServiceID: " << service->_serviceID << " Label: " << service->_label << " Current Channel " << service->_currentChannel << endl;
        _session.add(move(service));
    });*/

    _session.flush();
}

void CRadioServer::newRadioChannel(uint32_t serviceId, string serviceName, string channelID)
{
    stringstream stream;
    stream << hex << serviceId;
    string public_serviceId = stream.str();

    log("notice", "[WaveriderGUI::newRadioChannel] " + public_serviceId + ", " + serviceName + ", " + channelID);

    // Save this also on the Serverside to store into a database instance
    //CService *newService = new CService();
    dbo::Transaction transaction{_session};

    dbo::ptr<CService> newService = _session.add(std::unique_ptr<CService>{new CService()});
    newService.modify()->_serviceID = public_serviceId;
    newService.modify()->_label = serviceName;
    newService.modify()->_currentChannel = channelID;
    newService.modify()->_type = "dab+";
    
    // Dummy Data
    // Does country Exists?
    typedef dbo::collection< dbo::ptr<CDBOCountry> > AllCountries;
    AllCountries country = _session.find<CDBOCountry>().where("code == ?").bind("DE");
    dbo::ptr<CDBOCountry> searchedCountry;
    for(AllCountries::const_iterator it=country.begin(); it != country.end(); ++it)
    {
        dbo::ptr<CDBOCountry> s = *it;
        if (s->code == "DE")
        {
            searchedCountry = s;
            break;
        }
    }

    if (country.size() == 0)
    {
        searchedCountry = _session.add(std::unique_ptr<CDBOCountry>{new CDBOCountry()});
        searchedCountry.modify()->code = "DE";
        searchedCountry.modify()->name = "Deutschland";
    }

    dbo::ptr<CDBOInternetChannel> newAddService = _session.add(std::unique_ptr<CDBOInternetChannel>{new CDBOInternetChannel()});

    newAddService.modify()->language = "deutsch";
    newAddService.modify()->languagecode = "DE";
    newAddService.modify()->name = serviceName;
    newAddService.modify()->service = newService;
    newAddService.modify()->state = "Hamburg";
    newAddService.modify()->stationuuid = this->generateUUID();
    newAddService.modify()->tags = "dab+";
    newAddService.modify()->url = "";
    newAddService.modify()->url_resolved = "";
    newAddService.modify()->votes = 0;
    newAddService.modify()->bitrate = -1;
    newAddService.modify()->codec = "AAC";
    newAddService.modify()->country = searchedCountry;
    newAddService.modify()->favicon = "-1";
    newAddService.modify()->homepage = "";

    newService.modify()->_internetChannel = newAddService;

    _foundedChannels.push_back(newService);

    postRadioEvent(RadioEvent(serviceId, serviceName, channelID));
}

void CRadioServer::newEnsemble(string name) 
{
    log("notice", "[CRadioServer] ... new Ensemble says the backend ");
    /* WMenuItem* item = mChannelMenu->addItem(name, make_unique<WTextArea>(name));

    WApplication::instance()->triggerUpdate();*/
}

void CRadioServer::getDABChannels()
{
    log("notice", "[CRadioServer] Get Channels from DB ...");
    
    try
    {
        dbo::Transaction transaction{_session};

        typedef dbo::collection< dbo::ptr<CService> > Channels;
        //Channels channels = _session.find<CService>().orderBy("id");
        Channels channels = _session.find<CService>().where("type != ?").bind("web");
    
        for(Channels::const_iterator it=channels.begin(); it != channels.end(); ++it)
        {
            dbo::ptr<CService> s = *it;

            log("notice", "[CRadioServer] Load channel: " + s->_serviceID + ", " + s->_label + ", " + s->_currentChannel + ";");

            uint32_t serviceid = static_cast<uint32_t>(stoul(s->_serviceID.c_str(), NULL, 16));

            postRadioEvent(RadioEvent(serviceid, s->_label, s->_currentChannel));
        }
    }
    catch (dbo::Exception e)
    {
        // Database not exists. Do scan.
    }
}

void CRadioServer::getWebChannels(InternetChannelScroll action)
{
    try
    {
        dbo::Transaction transaction{_session};

        typedef dbo::collection< dbo::ptr<CService> > Channels;
        Channels channels;
        
        switch (action)
        {
            case InternetChannelScroll::nothing:
            {
                channels = _session.find<CService>().where("type == ?").bind("web");    
            } 
            break;
            case InternetChannelScroll::forward:
            {
                int last = _lastWebIDFromDBs.top();
                channels = _session.find<CService>().where("type == ? and id >= ?").bind("web").bind(last); 
            }
            break;
            case InternetChannelScroll::prev:
            {
                //int prev = _lastWebIDFromDBs.top();
                if (!_lastWebIDFromDBs.empty())
                    _lastWebIDFromDBs.pop();
                if (!_lastWebIDFromDBs.empty())
                    _lastWebIDFromDBs.pop();

                int prev = _lastWebIDFromDBs.top();
                //_lastWebIDFromDBs.pop();
                channels = _session.find<CService>().where("type == ? and id >= ?").bind("web").bind(prev); 
            }
            break;
        }

        //Channels channels = _session.find<CService>().where("type == ? and id > ?").bind("web").bind("3");
        //Channels channels = _session.find<CService>().where("id == ?").bind("9552");
        /* Channels channels = _session.query<dbo::ptr<CService>>("select t.id,t.version,t.serviceID,t.Label,t.Channel,t.Type,t.URL from (select * from servicetable left join internetchannels on servicetable.id=internetchannels.id) as t left join countries on t.country_id=countries.id").where("code = ?").bind("DE");*/
    
        int counter = 0;
        string lastStation = "";
        for(Channels::const_iterator it=channels.begin(); it != channels.end(); ++it)
        {
            dbo::ptr<CService> s = *it;

            postRadioEvent(RadioEvent(s->_label, s->_url));

            if (action == InternetChannelScroll::nothing && counter == 0)
            {
                // save the first web UUUID from DB
                int _lastWebIDFromDB = _session.query<int>("select id from internetchannels").where("stationuuid == ?").bind(s->_internetChannel->stationuuid);
                _lastWebIDFromDBs.push(_lastWebIDFromDB);
            }

            counter++;

            if (counter == 12)
            {
                lastStation = s->_internetChannel->stationuuid;
                postRadioEvent(RadioEvent(EventAction::createNextLink));
                
                break;     
            }
        }

        if (counter < 12)
        {
            postRadioEvent(RadioEvent(EventAction::removeNextLink));    
        }

        int _lastWebIDFromDB = _session.query<int>("select id from internetchannels").where("stationuuid == ?").bind(lastStation);
        _lastWebIDFromDBs.push(_lastWebIDFromDB);

        if (_lastWebIDFromDBs.size() == 2)
        {
            postRadioEvent(RadioEvent(EventAction::removePrevLink));    
        }
        if (_lastWebIDFromDBs.size() > 2)
        {
            postRadioEvent(RadioEvent(EventAction::createPrevLink));    
        }
    }
    catch (dbo::Exception e)
    {
        // Database not exists. Do scan.
    }
}

void CRadioServer::getBouquets()
{
    try
    {
        dbo::Transaction transaction{_session};

        typedef dbo::collection< dbo::ptr<CDBOBouquet> > Bouquets;
        Bouquets bouquets = _session.find<CDBOBouquet>();
    
        for(Bouquets::const_iterator it=bouquets.begin(); it != bouquets.end(); ++it)
        {
            dbo::ptr<CDBOBouquet> b = *it;

            postRadioEvent(RadioEvent(b->name, EventAction::bouquetFound));
        }
    }
    catch (dbo::Exception e)
    {
        // Database not exists. Do scan.
    }
}

void CRadioServer::loadChannelsInBouquet(string bouquetName)
{
    try
    {
        dbo::Transaction transaction{_session};
        dbo::ptr<CDBOBouquet> bouquet = _session.find<CDBOBouquet>().where("name = ?").bind(bouquetName);

        for(auto it = begin(bouquet->services); it != end(bouquet->services); ++it)
        {
            dbo::ptr<CService> s = *it;

            if (s->_type == "dab+")
            {
                uint32_t serviceid = static_cast<uint32_t>(stoul(s->_serviceID.c_str(), NULL, 16));

                postRadioEvent(RadioEvent(serviceid, s->_label, s->_currentChannel));
            }
            else if (s->_type == "web")
            {
                postRadioEvent(RadioEvent(s->_label, s->_url));
            }
        }

        postRadioEvent(RadioEvent(EventAction::backToBouquets));
    }
    catch (dbo::Exception e)
    {
        // Database not exists. Do scan.
    }    
}

void CRadioServer::getSettings()
{
    dbo::ptr<CSettings> s = _settings();

    postSettingEvent(SettingsEvent(
        s->_radioDevice, 
        s->_ipaddress,
        s->_port,
        s->_recordPath,
        s->_radioBrowserURL,
        s->_doNotReEncodeIChannels,
        s->_streaming,
        s->_streamingAddress,
        s->_streamingFormat,
        s->_recordFormat,
        s->_metadata,
        s->_saveMetadata,
        SettingsAction::loadedSettings));
}

void CRadioServer::saveSettings(string device, string ipaddress, string port, string recordPath, string radioBrowserURL, string donotreencode, string streaming, string streamingAddress, string streamingFormat, string recordFormat, string metadata, string saveMetadata)
{
    _refreshSettings = true;

    dbo::ptr<CSettings> s = _settings();
    dbo::Transaction transaction{_session};

    s.modify()->setRadioDevice(device);
    s.modify()->setRecordPath(recordPath);
    s.modify()->setIPAddress(ipaddress);
    s.modify()->setPort(port);
    s.modify()->setRadioBrowserURL(radioBrowserURL);

    s.modify()->setDoNotReEncodeIChannels(donotreencode);
    s.modify()->setStreaming(streaming);
    s.modify()->setStreamingAddress(streamingAddress);
    s.modify()->setStreamingFormat(streamingFormat);
    s.modify()->setRecordFormat(recordFormat);

    s.modify()->setRetrieveMetadata(metadata);
    s.modify()->setSaveMetadata(saveMetadata);

    _session.flush();

    _refreshSettings = true;

    postSettingEvent(SettingsEvent(recordPath, SettingsAction::newSettings));
}

void CRadioServer::audioDataIsAvailable()
{
    log("notice", "[CRadioServer] audioDataIsAvailable, we can play audio ...");

    _channelChange = false;

    _jplayerStreamer->prepareStreaming();

    postRadioEvent(RadioEvent(EventAction::receiveAudio));
}

void CRadioServer::setChannel(uint32_t serviceid, string serviceLabel, string channel)
{
    _channelChange = true;
    _currentPlayingStationType = CurrentPlayingStationType::dab;

    if (_jplayerStreamer->isStreaming())
    {
        _jplayerStreamer->stopStreaming();
    }

    _jplayerStreamer->setChannel(serviceid, serviceLabel, channel);

    _radioController->openDevice();
    this_thread::sleep_for(chrono::seconds(3));
    _radioController->play(channel, serviceLabel, serviceid);

    _playingChannel = serviceLabel;
    _playingURL = "";
    _playingChannel.erase(_playingChannel.find_last_not_of(" ")+1);

    postRadioEvent(RadioEvent(serviceid, serviceLabel, channel, EventAction::channelChange));
}

void CRadioServer::setWebChannel(string serviceLabel, string url)
{
    _channelChange = true;
    _currentPlayingStationType = CurrentPlayingStationType::web;

    if (_jplayerStreamer->isStreaming())
    {
        _jplayerStreamer->stopStreaming();
    }

    _jplayerStreamer->setWebChannel(serviceLabel, url);
    _radioController->play(serviceLabel, url);

    //_jplayerStreamer->prepareWebStreaming();

    _playingChannel = serviceLabel;
    _playingURL = url;
    _playingChannel.erase(_playingChannel.find_last_not_of(" ")+1);   

    postRadioEvent(RadioEvent(0x0, serviceLabel, "0x00", EventAction::channelChange));
}

string CRadioServer::getInternetChannelCodec()
{
    if (_playingURL.length() == 0)
    {
        return "";
    }

    dbo::Transaction transaction{_session};

    dbo::ptr<CDBOInternetChannel> channels = _session.find<CDBOInternetChannel>().where("url=?").bind(_playingURL);

    string codec_name = channels.get()->codec;
    transform(codec_name.begin(), codec_name.end(), codec_name.begin(), 
    [](unsigned char c){ 
        return tolower(c); 
    });

    return codec_name;
}

void CRadioServer::reactivateRecordingChannel()
{
    _jplayerStreamer->sendAudioHeaderAgain();

    postRadioEvent(RadioEvent(
        _jplayerStreamer->getPlayingServiceID(),
        _jplayerStreamer->getPlayingServiceName(),
        _jplayerStreamer->getPlayingChannelID(),
        EventAction::channelChange));
}

void CRadioServer::stop()
{
    if (_jplayerStreamer->isStreaming())
    {
        _jplayerStreamer->stopStreaming();
    }

    _radioController->stop();

    _playingChannel = "";
    _playingURL = "";
    _radioController->setRecordFlag(false);

    _currentPlayingStationType = CurrentPlayingStationType::noservice;
}

void CRadioServer::webStop()
{
    if (_jplayerStreamer->isStreaming())
    {
        _jplayerStreamer->stopStreaming();
    }

    _radioController->webStop(); 

    _playingChannel = "";
    _playingURL = "";
    _radioController->setRecordFlag(false);

    _currentPlayingStationType = CurrentPlayingStationType::noservice;
}

void CRadioServer::shutup()
{
    log("error", "[CRadioServer] Emergency stop ...");

    if (isPlaying())
    {
        log("notice", "Stopping dab and web ...");

        switch (_currentPlayingStationType)
        {
            case dab: stop();
            break;
            case web: webStop();
            break;
        }
    }
    else
        log("notice", "[CRadioServer] Nothing to shutdown ...");
}

void CRadioServer::updateMOT(mot_file_t mot_file)
{
    string mimetype = "";
    switch (mot_file.content_sub_type)
    {
        case 0: mimetype = "image/GIF";
                break;
        case 1: mimetype = "image/JPEG";
                break;
        case 2: mimetype = "image/BMP";
                break;
        default:    mimetype = "image/PNG";
                    break;
    }

    this->removeEntryPoint(mot_resource_path);

    random_device rd;
    default_random_engine eng(rd());
    uniform_int_distribution<int> distr(1, 100);

    stringstream s; 
    s << eng;

    mot_resource_path = "/dab/mot_" + s.str();

    _motImage = make_unique<WMemoryResource>(mimetype);
    _motImage->setData(mot_file.data);
    this->addResource(_motImage.get(), mot_resource_path);

    postRadioEvent(RadioEvent(0, mot_resource_path, "", EventAction::motChange));
}

void CRadioServer::updateSNR(int snr)
{
    double percent = 1.0*snr / 16 * 100;

    if (percent <= 0)
    {
        postRadioEvent(RadioEvent("signal_null.png", EventAction::signalStrength));  
    }
    else if (percent > 0 && percent <= 20)
    {
        postRadioEvent(RadioEvent("signal_1.png", EventAction::signalStrength));
    }
    else if (percent > 20 && percent <= 40)
    {
        postRadioEvent(RadioEvent("signal_2.png", EventAction::signalStrength));
    }
    else if (percent > 40 && percent <= 60) 
    {
        postRadioEvent(RadioEvent("signal_3.png", EventAction::signalStrength));
    }
    else if (percent > 60 && percent <= 80)
    {
        postRadioEvent(RadioEvent("signal_4.png", EventAction::signalStrength));
    }
    else if (percent > 80 && percent <= 100)
    {
        postRadioEvent(RadioEvent("signal_5.png", EventAction::signalStrength));
    }
}

void CRadioServer::updateSearchingChannel(string channel, string stationCount)
{
    postRadioEvent(RadioEvent(channel, stationCount, EventAction::channelScan));
}

void CRadioServer::recordStream()
{
    _radioController->setRecordToDir(_settings()->_recordPath);
    _radioController->setRecordFromChannel(_playingChannel);

    if (_radioController->recordFlag())
    {
        _radioController->setRecordFlag(false);
    }
    else
    {
        _radioController->setRecordFlag(true);
    } 
}

bool CRadioServer::isRecording()
{
    if (_radioController->recordFlag())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CRadioServer::isPlaying()
{
    if (_radioController->playFlag())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void CRadioServer::setInternetChannel(string link)
{
    if (_streamingServer != NULL) 
    {
        _streamingServer->setInternetChannel(link);
    }
}

void CRadioServer::addWebChannel(string name, string url)
{
    log("notice", "Add Web Channel: Station Name: " + name + ", URL: " + url);

    dbo::Transaction transaction{_session};

    auto service = make_unique<CService>();
    service->_serviceID = "0x00";
    service->_label = name;
    service->_currentChannel = "0x00";

    // Type
    service->_type = "web";
    service->_url = url;

    _session.add(move(service));
    _session.flush();
}

void CRadioServer::soundcardFound(string name)
{
    postRadioEvent(RadioEvent(-1, name, EventAction::soundcardFound));
}

void CRadioServer::addCountry(string code)
{
    //auto country = dbo::make_ptr<CDBOCountry>();
    dbo::ptr<CDBOCountry> country = _session.add(std::unique_ptr<CDBOCountry>{new CDBOCountry()});
    country.modify()->code = code;
    country.modify()->name = "";

    _foundedCountries.push_back(country); 
}

dbo::ptr<CDBOCountry> CRadioServer::getCountryPtr(string code)
{
    dbo::Transaction transaction{_session};
    vector<dbo::ptr<CDBOCountry>>::iterator it;
    for (it = _foundedCountries.begin(); it != _foundedCountries.end(); ++it)
    {
        dbo::ptr<CDBOCountry> dbc = *it;
        
        if (dbc->code == code)
            return dbc;
    }

    log("error", "Country not found in the database!");

    return nullptr;
}

void CRadioServer::addWebChannel(string stationuuid, string name, string url, string url_resolved, string homepage, string favicon, string tags, string country, string countrycode, string state, string language, string languagecode, int votes, string codec, int bitrate)
{
    dbo::ptr<CDBOCountry> c = getCountryPtr(countrycode);
    
    c.modify()->name = country;

    dbo::ptr<CDBOInternetChannel> webchannel = _session.add(unique_ptr<CDBOInternetChannel>{new CDBOInternetChannel()});

    webchannel.modify()->stationuuid = stationuuid;
    webchannel.modify()->name = name;
    webchannel.modify()->url = url;
    webchannel.modify()->url_resolved = url_resolved;
    webchannel.modify()->homepage = homepage;
    webchannel.modify()->favicon = favicon;
    webchannel.modify()->tags = tags;
    webchannel.modify()->country = c;
    webchannel.modify()->state = state;
    webchannel.modify()->language = language;
    webchannel.modify()->languagecode = languagecode;
    webchannel.modify()->votes = votes;
    webchannel.modify()->codec = codec;
    webchannel.modify()->bitrate = bitrate;

    dbo::ptr<CService> service = _session.add(unique_ptr<CService>{new CService()});
    service.modify()->_serviceID = "0000";
    service.modify()->_type = "web";
    service.modify()->_label = name;
    service.modify()->_url = url;
    //service.modify()->_countrycode = countrycode;
    service.modify()->_currentChannel = "00X";

    webchannel.modify()->service = service;

    dbo::Transaction transaction{_session};
    _session.flush();
}

void CRadioServer::tellCountryCount(int countedCountries)
{
    postRadioEvent(RadioEvent(countedCountries, "", "", EventAction::internetCountryCount));
}
        
void CRadioServer::tellCountryScan(int numberOfCountry, string country, string countryCode)
{
    postRadioEvent(RadioEvent(numberOfCountry, country, countryCode, EventAction::internetCountryScan));
}

void CRadioServer::scan_internet_finished()
{
    postRadioEvent(RadioEvent(EventAction::internetScanFinished));
}

void CRadioServer::getAllWebChannels()
{
    dbo::Transaction transaction{_session};
    dbo::collection<dbo::ptr<CDBOInternetChannel>> channels = _session.find<CDBOInternetChannel>();
    map<string, string> export_channels;

    for (auto it = begin(channels); it != end(channels); ++it)
    {
        dbo::ptr<CDBOInternetChannel> ichannel = *it;
        export_channels[ichannel->stationuuid] = ichannel->name;
    }

    postBouquetEditorEvent(BouquetEditorEvent(export_channels, BouquetEditorEventAction::savedChannels));
}

void CRadioServer::getSavedChannelsOrderByVotes()
{
    dbo::Transaction transaction{_session};
    typedef Wt::Dbo::ptr<CDBOInternetChannel> IChannelPtr;
    typedef Wt::Dbo::collection<IChannelPtr> votesPtr;

    dbo::Query<IChannelPtr> query = _session.query<IChannelPtr>("select i from internetchannels i").orderBy("votes");
    votesPtr db_votes = query.resultList();

    map<string, string> export_channels;

    for (auto it = begin(db_votes); it != end(db_votes); ++it)
    {
        dbo::ptr<CDBOInternetChannel> ichannel = *it;
        export_channels[ichannel->stationuuid] = ichannel->name;
    }

    postBouquetEditorEvent(BouquetEditorEvent(export_channels, BouquetEditorEventAction::savedChannels));    
}

void CRadioServer::getSavedCountries()
{
    dbo::Transaction transaction{_session};
    typedef dbo::collection< dbo::ptr<CDBOCountry> > dbCountries;
    dbCountries db_countries = _session.find<CDBOCountry>();

    vector<string> countries;

    for (auto it = begin(db_countries); it != end(db_countries); ++it )
    {
        dbo::ptr<CDBOCountry> c = *it;
        countries.push_back(c->name);
    }
    //countries.push_back("country 1");
    //countries.push_back("country 2");

    postBouquetEditorEvent(BouquetEditorEvent(countries, BouquetEditorEventAction::savedCountries));
}

void CRadioServer::getSavedLanguages()
{
    dbo::Transaction transaction{_session};
    typedef Wt::Dbo::ptr<CDBOInternetChannel> IChannelPtr;
    typedef Wt::Dbo::collection<IChannelPtr> languagePtr;

    dbo::Query<IChannelPtr> query = _session.query<IChannelPtr>("select i from internetchannels i").groupBy("language");
    languagePtr languages = query.resultList();

    vector<string> db_languages;

    for (auto it = begin(languages); it != end(languages); ++it)
    {
        dbo::ptr<CDBOInternetChannel> c = *it;
        db_languages.push_back(c->language);
    }

    postBouquetEditorEvent(BouquetEditorEvent(db_languages, BouquetEditorEventAction::savedLanguages));    
}

void CRadioServer::getSavedTags()
{
    dbo::Transaction transaction{_session};
    typedef Wt::Dbo::ptr<CDBOInternetChannel> IChannelPtr;
    typedef Wt::Dbo::collection<IChannelPtr> tagsPtr;

    dbo::Query<IChannelPtr> query = _session.query<IChannelPtr>("select i from internetchannels i").groupBy("tags");
    tagsPtr tags = query.resultList();

    map<string, string> db_tags;

    for (auto it = begin(tags); it != end(tags); ++it)
    {
        dbo::ptr<CDBOInternetChannel> c = *it;
        //db_tags.push_back(c->tags);
        vector<string> tags = MiscTools::SplitString(c->tags, ',');
        for (auto it2 = begin(tags); it2 != end(tags); ++it2)
        {
            //cout << "Tag: " << *it2 << endl;
            db_tags[*it2] = *it2;
        }
    }

    // map to vector
    vector<string> tags_filter;
    for (auto it = begin(db_tags); it != end(db_tags); ++it)
    {
        tags_filter.push_back(it->second);
    }

    postBouquetEditorEvent(BouquetEditorEvent(tags_filter, BouquetEditorEventAction::savedTags));
}

void CRadioServer::getSavedCodecs()
{
    dbo::Transaction transaction{_session};
    typedef Wt::Dbo::ptr<CDBOInternetChannel> IChannelPtr;
    typedef Wt::Dbo::collection<IChannelPtr> codecsPtr;

    dbo::Query<IChannelPtr> query = _session.query<IChannelPtr>("select i from internetchannels i").groupBy("codec");
    codecsPtr codecs = query.resultList();

    vector<string> db_codecs;

    for (auto it = begin(codecs); it != end(codecs); ++it)
    {
        dbo::ptr<CDBOInternetChannel> c = *it;
        db_codecs.push_back(c->codec);
    }

    postBouquetEditorEvent(BouquetEditorEvent(db_codecs, BouquetEditorEventAction::savedCodecs));
}

void CRadioServer::getSavedCountryCodeExact()
{
    dbo::Transaction transaction{_session};
    typedef dbo::collection< dbo::ptr<CDBOCountry> > dbCountries;
    dbCountries db_countries = _session.find<CDBOCountry>();

    vector<string> countries;

    for (auto it = begin(db_countries); it != end(db_countries); ++it )
    {
        dbo::ptr<CDBOCountry> c = *it;
        countries.push_back(c->code);
    }

    postBouquetEditorEvent(BouquetEditorEvent(countries, BouquetEditorEventAction::savedExactCountryCodes));    
}

void CRadioServer::getWebChannelsFromCountryCodeExact(string fromCode)
{
    dbo::Transaction transaction{_session};
    typedef dbo::collection< dbo::ptr<CDBOCountry> > dbCountries;
    dbCountries db_countries = _session.find<CDBOCountry>().where("code = ?").bind(fromCode);

    map<string, string> export_channels;

    for (auto it = begin(db_countries); it != end(db_countries); ++it )
    {
        dbo::ptr<CDBOCountry> c = *it;
        dbo::collection<dbo::ptr<CDBOInternetChannel>> channels = c->internetchannels;

        for (auto cit = begin(channels); cit != end(channels); ++cit)
        {
            dbo::ptr<CDBOInternetChannel> ichannel = *cit;
            
            export_channels[ichannel->stationuuid] = ichannel->name;
        }

        break;        
    }

    postBouquetEditorEvent(BouquetEditorEvent(export_channels, BouquetEditorEventAction::savedChannels));
}

void CRadioServer::getWebChannelsFromCountry(string fromCountry)
{
    dbo::Transaction transaction{_session};
    typedef dbo::collection< dbo::ptr<CDBOCountry> > dbCountries;
    dbCountries db_countries = _session.find<CDBOCountry>().where("name = ?").bind(fromCountry);

    map<string, string> export_channels;

    for (auto it = begin(db_countries); it != end(db_countries); ++it )
    {
        dbo::ptr<CDBOCountry> c = *it;
        
        dbo::collection<dbo::ptr<CDBOInternetChannel>> channels = c->internetchannels;

        for (auto cit = begin(channels); cit != end(channels); ++cit)
        {
            dbo::ptr<CDBOInternetChannel> ichannel = *cit;
            
            export_channels[ichannel->stationuuid] = ichannel->name;
        }

        break;        
    }

    postBouquetEditorEvent(BouquetEditorEvent(export_channels, BouquetEditorEventAction::savedChannels));
}

void CRadioServer::getWebChannelsFromCodec(string codec)
{
    dbo::Transaction transaction{_session};
    typedef dbo::collection< dbo::ptr<CDBOInternetChannel> > dbChannels;
    dbChannels db_channels = _session.find<CDBOInternetChannel>().where("codec = ?").bind(codec);

    map<string, string> export_channels;

    for (auto it = begin(db_channels); it != end(db_channels); ++it )
    {
        dbo::ptr<CDBOInternetChannel> c = *it;
        
        export_channels[c->stationuuid] = c->name;  
    }

    postBouquetEditorEvent(BouquetEditorEvent(export_channels, BouquetEditorEventAction::savedChannels));    
}

void CRadioServer::getWebChannelsFromLanguage(string language)
{
    dbo::Transaction transaction{_session};
    typedef dbo::collection< dbo::ptr<CDBOInternetChannel> > dbLanguages;
    dbLanguages db_channels = _session.find<CDBOInternetChannel>().where("language = ?").bind(language);

    map<string, string> export_channels;

    for (auto it = begin(db_channels); it != end(db_channels); ++it )
    {
        dbo::ptr<CDBOInternetChannel> c = *it;
        export_channels[c->stationuuid] = c->name;  
    }

    postBouquetEditorEvent(BouquetEditorEvent(export_channels, BouquetEditorEventAction::savedChannels));    
}

void CRadioServer::getWebChannelsFromTags(string tag)
{
    dbo::Transaction transaction{_session};
    typedef dbo::collection< dbo::ptr<CDBOInternetChannel> > dbTags;
    dbTags db_tags = _session.find<CDBOInternetChannel>().where("tags like ?").bind("%"+tag+"%");

    map<string, string> export_channels;

    for (auto it = begin(db_tags); it != end(db_tags); ++it)
    {
        dbo::ptr<CDBOInternetChannel> c = *it;
        export_channels[c->stationuuid] = c->name;  
    }

    postBouquetEditorEvent(BouquetEditorEvent(export_channels, BouquetEditorEventAction::savedChannels));
}

void CRadioServer::getSavedDABChannels()
{
    dbo::Transaction transaction{_session};
    typedef dbo::collection< dbo::ptr<CService> > dbDabChannels;
    dbDabChannels db_dabplus = _session.find<CService>().where("Type = ?").bind("dab+");

    map<string, string> export_channels;

    for (auto it = begin(db_dabplus); it != end(db_dabplus); ++it)
    {
        dbo::ptr<CService> c = *it;
        
        export_channels[c->_internetChannel->stationuuid] = c->_label;  
    }

    postBouquetEditorEvent(BouquetEditorEvent(export_channels, BouquetEditorEventAction::savedDABChannels));
}

void CRadioServer::saveBouquetsInDB(map<string, map<string,string>> bouquets)
{
    dbo::Transaction transaction{_session};
    typedef dbo::collection< dbo::ptr<CDBOBouquet> > dbBouquets;
    dbBouquets db_bouquets = _session.find<CDBOBouquet>();
    list<string> toDeleted;

    // Does a bouquet exists, which can be deleted?
    for (auto it = begin(db_bouquets); it != end(db_bouquets); ++it)
    {
        dbo::ptr<CDBOBouquet> b = *it;
        bool toDelete = true;

        for (auto it2 = begin(bouquets); it2 != end(bouquets); ++it2)
        {
            if (b->name == it2->first)
            {
                toDelete = false;
            }
        }

        if (toDelete)
        {
            toDeleted.push_front(b->name);
        }
    } 

    // delete not used bouquets
    for (auto it = begin(toDeleted); it != end(toDeleted); ++it)
    {
        dbo::ptr<CDBOBouquet> bq = _session.find<CDBOBouquet>().where("name = ?").bind(*it);
        bq.remove();
    }

    for (auto it = begin(bouquets); it != end(bouquets); ++it)
    {
        dbo::ptr<CDBOBouquet> bq;
        if (_session.find<CDBOBouquet>().where("name = ?").bind(it->first).resultList().size()==0)
        {
            bq = _session.add(std::unique_ptr<CDBOBouquet>{new CDBOBouquet()});
        }
        else
        {
            bq = _session.find<CDBOBouquet>().where("name = ?").bind(it->first);
        }

        bq.modify()->name = it->first;

        // search channels for a delete
        list<dbo::ptr<CService>> toDeletedServices;
        for (auto it2 = begin(bq->services); it2 != end(bq->services); ++it2)
        {
            dbo::ptr<CService> s = *it2;
            bool toDelete = true;

            for (auto it3 = begin(it->second); it3 != end(it->second); ++it3)
            {
                if (s->_internetChannel->stationuuid == it3->first)
                {
                    toDelete = false;
                }
            }    

            if (toDelete)
            {
                toDeletedServices.push_front(s);
            }
        }

        for (auto it2 = begin(toDeletedServices); it2 != end(toDeletedServices); ++it2)
        {
            bq.modify()->services.erase(*it2);
        }
        
        //add exists services to this bouquet
        for (auto it2 = begin(it->second); it2 != end(it->second); ++it2)
        {
           // identify personal added radio station ...
           if (it2->first[0] == '-')
           {
               // New added Channel
               dbo::ptr<CService> newService = _session.add(std::unique_ptr<CService>{new CService()});
               newService.modify()->_serviceID = "0x0";
               newService.modify()->_label = it2->second;
               newService.modify()->_currentChannel = "0x0XX";
               newService.modify()->_type = "web";

               vector<string> data = MiscTools::SplitString(it2->first, ';');

               dbo::ptr<CDBOCountry> searchedCountry = _session.find<CDBOCountry>().where("code = ?").bind("DE");
               dbo::ptr<CDBOInternetChannel> addedChannel =  _session.add(std::unique_ptr<CDBOInternetChannel>{new CDBOInternetChannel()});

               addedChannel.modify()->language = "deutsch";
               addedChannel.modify()->languagecode = "DE";
               addedChannel.modify()->name = it2->second;
               addedChannel.modify()->service = newService;
               addedChannel.modify()->state = "";
               addedChannel.modify()->stationuuid = this->generateUUID();
               addedChannel.modify()->tags = "web";
               addedChannel.modify()->url = data[1];
               addedChannel.modify()->url_resolved = data[1];
               addedChannel.modify()->votes = 0;
               addedChannel.modify()->bitrate = -1;
               addedChannel.modify()->codec = "UNKNOWN";
               addedChannel.modify()->country = searchedCountry;
               addedChannel.modify()->favicon = data[2];
               addedChannel.modify()->homepage = "";

               newService.modify()->_internetChannel = addedChannel;
               newService.modify()->_url = data[1];

               bq.modify()->services.insert(newService);
           }
           else 
           {
               dbo::ptr<CDBOInternetChannel> dbo_channel = _session.find<CDBOInternetChannel>().where("stationuuid = ?").bind(it2->first);

               //dbo::ptr<CService> service = dbo_channel->service;        
               dbo::ptr<CService> service = _session.find<CService>().where("id = ?").bind(dbo_channel.id());

               //service.modify()->_bouquets.insert(bq);
               if (bq->services.find().where("id = ?").bind(service.id()).resultList().size() == 0)
               {
                    bq.modify()->services.insert(service);
               }
            }
        }
    }

    _session.flush();

    postBouquetEditorEvent(BouquetEditorEvent(BouquetEditorEventAction::newBouquets));
}

void CRadioServer::getBouquetsInDB()
{
    dbo::Transaction transaction{_session};
    map<string, map<string,string>> bouquets;

    dbo::collection<dbo::ptr<CDBOBouquet>> bqs = _session.find<CDBOBouquet>();
    
    for (auto it=begin(bqs); it != end(bqs); ++it)
    {
        map<string, string> channels;
        
        dbo::ptr<CDBOBouquet> b = *it;

        for (auto it2 = begin(b->services); it2 != end(b->services); ++it2)
        {
            dbo::ptr<CService> s = *it2;
            channels[s->_internetChannel->stationuuid] = s->_label;
        }

        bouquets[b->name] = channels;
    }

    postBouquetEditorEvent(BouquetEditorEvent(bouquets, BouquetEditorEventAction::bouquetsLoaded));
}

string CRadioServer::generateUUID()
{
    static random_device dev;
    static mt19937 rng(dev());

    uniform_int_distribution<int> dist(0, 15);

    const char *v = "0123456789abcdef";
    const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

    string res;
    for (int i = 0; i < 16; i++) {
        if (dash[i]) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}

void CRadioServer::updateNewTitle(string newTitle)
{
    postRadioEvent(RadioEvent(newTitle, EventAction::newTitle));
    this->saveArtistAndTitle(newTitle);
}

// If no cover was found show the logo of the RadioStation (Internetradio)
string CRadioServer::getRadioStationFavIcon()
{
    dbo::Transaction transaction{_session};
    
    try
    {
        Dbo::ptr<CDBOInternetChannel> ic = _session.find<CDBOInternetChannel>().where("name = ?").bind(_playingChannel);

        return ic->favicon;
    }
    catch (Dbo::NoUniqueResultException e)
    {  
        if (_playingChannel.length() != 0)
        {
            Dbo::ptr<CDBOInternetChannel> ic = _session.find<CDBOInternetChannel>().where("url = ?").bind(_playingURL);

            return ic->favicon;
        }
        else
        {
            // Nothing to do just return an empty string as url
            return "";
        }
    }

    return "";  // nothing found
}

void CRadioServer::saveRecordStartTime()
{
    _TimePointRecordStart = chrono::system_clock::now();
}

void CRadioServer::saveArtistAndTitle(string title)
{
    int pos = title.find("Title:");

    if (_Artist.length() != 0 || _Songname.length() != 0)
    {
        // No motd was sended, save the last one if available
        if (_lastMotFile.data.size() > 0)
            saveMot(_lastMotFile);
        
        _Artist = "";
        _Songname = "";
    }

    _Artist = title.substr(14, pos - 15);
    _Songname = title.substr(pos + 7, title.length() - (pos + 7));
}

void CRadioServer::saveFilename(string fn)
{
    _RecordFilename = fn;
}

void CRadioServer::saveTimeOfRecordStart()
{
    _TimePointRecordStart = chrono::system_clock::now();
}

void CRadioServer::saveMot(mot_file_t mot_file)
{
    dbo::Transaction transaction{_session};

    dbo::ptr<CDBORecord> newRecord = _session.add(std::unique_ptr<CDBORecord>{new CDBORecord()});

    newRecord.modify()->artist = _Artist;
    newRecord.modify()->titlename = _Songname;
    newRecord.modify()->filename = _RecordFilename;
    newRecord.modify()->timestamp = _TimePointRecordStart;
    for (int i=0; i<mot_file.data.size(); i++) 
        newRecord.modify()->cover.push_back(mot_file.data[i]); 
    newRecord.modify()->type = mot_file.content_sub_type;

    _session.flush();

    // save, if no one is sending
    _lastMotFile.data.clear();
    _lastMotFile.content_sub_type = mot_file.content_sub_type;
    for (int i=0; i<mot_file.data.size(); i++) 
        _lastMotFile.data.push_back(mot_file.data[i]);

    _Artist = "";
    _Songname = "";
}

void CRadioServer::showContentOfRecordedFile(string filename)
{
    srand((unsigned)time(NULL)); // get rid of browser caching images

    // delete old ressources
    this->deleteResources("/recordedFile");

    dbo::Transaction transaction{_session};

    vector<vector<string>> content;

    dbo::collection<dbo::ptr<CDBORecord>> c = _session.find<CDBORecord>().where("filename=?").bind(filename);

    int lfdNr = 0;
    for (auto it=begin(c); it != end(c); ++it)
    {
        dbo::ptr<CDBORecord> r = *it;

        vector<string> v;
        time_t tt = chrono::system_clock::to_time_t(r.get()->timestamp);
        tm tm = *std::localtime(&tt); //GMT (UTC)
        stringstream ss;
        ss << put_time( &tm, "%d.%m.%Y %H:%M:%S");
        v.push_back(ss.str());
        
        v.push_back(r.get()->titlename);
        v.push_back(r.get()->artist);

        string mimetype = "";
        switch (r.get()->type)
        {
            case 0: mimetype = "image/GIF";
                    break;
            case 1: mimetype = "image/JPEG";
                    break;
            case 2: mimetype = "image/BMP";
                    break;
            default:    mimetype = "image/PNG";
                        break;
        }

        WMemoryResource *wm = new WMemoryResource(mimetype);
        wm->setData(r.get()->cover);
        string dr = "/recordedFile/" + to_string(rand()) + "_" + to_string(lfdNr);
        this->addResource(wm, dr);
        v.push_back(dr);

        content.push_back(v);
        lfdNr++;
    }   

    postRecordFileEvent(RecordFileEvent(content));
}

void CRadioServer::addResource(WResource *r, const string& path)
{
    WServer::addResource(r, path);

    _entryPoints.push_back(path);
}

void CRadioServer::deleteResources(string mainPath)
{
    vector<string> copy;

    for (auto it = begin(_entryPoints); it != end(_entryPoints); ++it)
    {
        string r = *it;

        if (r.find(mainPath) != string::npos)
        {
            removeEntryPoint(r);
        }
        else
        {
            copy.push_back(r);
        }
    }

    _entryPoints.clear();
    for (auto it = begin(copy); it != end(copy); ++it)
    {
        _entryPoints.push_back(*it);
    }
}

void CRadioServer::errorOccur(string message)
{
    postErrorEvent(ErrorEvent(message));
}

bool CRadioServer::getDoNotReencodeInternetChannels()
{
    return to_bool(_settings()->_doNotReEncodeIChannels);
}

AudioCompression CRadioServer::getRecordFormat()
{
    log("info", "[CRadioServer] using " + _settings()->_recordFormat + " for recording.");

    if (_settings()->_recordFormat == "AAC")
        return AudioCompression::AAC;
    else if (_settings()->_recordFormat == "ALAC")
        return AudioCompression::ALAC;
    else if (_settings()->_recordFormat == "FLAC")
        return AudioCompression::FLAC;
    else if (_settings()->_recordFormat == "MP2")
        return AudioCompression::MP2;
    else if (_settings()->_recordFormat == "MP3")
        return AudioCompression::MP3;
    else if (_settings()->_recordFormat == "VORBIS")
        return AudioCompression::VORBIS;
    else if (_settings()->_recordFormat == "WAV")
        return AudioCompression::WAV;
    else
        return AudioCompression::WAV;
}

string CRadioServer::getEndingForRecordingFile()
{
    if (_settings()->_recordFormat == "AAC")
        return "aac";
    else if (_settings()->_recordFormat == "ALAC")
        return "m4a";
    else if (_settings()->_recordFormat == "FLAC")
        return "flac";
    else if (_settings()->_recordFormat == "MP2")
        return "mp2";
    else if (_settings()->_recordFormat == "MP3")
        return "mp3";
    else if (_settings()->_recordFormat == "VORBIS")
        return "ogg";
    else if (_settings()->_recordFormat == "WAV")
        return "wav";
    else
        return "wav";
}

bool CRadioServer::getStreaming()
{
    return to_bool(_settings()->_streaming);
}

string CRadioServer::getStreamingAddress()
{
    return _settings()->_streamingAddress;
}

AudioCompression CRadioServer::getStreamingFormat()
{
    if (_settings()->_streamingFormat == "AAC")
        return AudioCompression::FLAC;
    else if (_settings()->_streamingFormat == "ALAC")
        return AudioCompression::ALAC;
    else if (_settings()->_streamingFormat == "FLAC")
        return AudioCompression::FLAC;
    else if (_settings()->_streamingFormat == "MP2")
        return AudioCompression::MP2;
    else if (_settings()->_streamingFormat == "MP3")
        return AudioCompression::MP3;
    else if (_settings()->_streamingFormat == "VORBIS")
        return AudioCompression::VORBIS;
    else if (_settings()->_streamingFormat == "WAV")
        return AudioCompression::WAV;
    else
        return AudioCompression::WAV;
}

bool CRadioServer::getRetrieveMetadata()
{
    return to_bool(_settings()->_metadata);
}

bool CRadioServer::getSaveMetadata()
{
    return to_bool(_settings()->_saveMetadata);
}

bool CRadioServer::isInternetChannelPlaying()
{
    return (_playingURL.length() != 0) ? true : false;
}

void CRadioServer::setLogger(WLogger& logger)
{
    _logger = logger;
}

void CRadioServer::log(string type, string message)
{
    // Add an entry
    //WLogEntry entry = _logger.entry(type);
    WLogEntry entry = this->logger().entry(type);
    
    entry << WLogger::timestamp << Wt::WLogger::sep
      << '[' << type << ']' << Wt::WLogger::sep
      << message;
}