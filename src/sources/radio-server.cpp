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

CRadioServer::CRadioServer(int argc, char *argv[], const std::string &wtConfigurationFile)
:   WServer(argc, argv, wtConfigurationFile)
{
    cout << "Radio Server is listening" << endl;

    _radioController = make_shared<CRadioController>(this);

    /* CWavStreamerResource *source = new CWavStreamerResource(_radioController);
    source->setChannel(0x1258, "Klassik R. Movie", "5D");
    this->addResource(source, "/dab/0x1258"); */

    _jplayerStreamer = new CJPlayerStreamerResource(_radioController);
    this->addResource(_jplayerStreamer, "/dab/jplayer");

    _dbConnector = make_unique<dbo::backend::Sqlite3>("waverider.db"); 
    _session.setConnection(std::move(_dbConnector));

    _session.mapClass<CService>("servicetable");
    
    // prepare dummy mot Image
    ifstream input(this->appRoot() + "mot/test-pattern-640.png", std::ios::binary );
    // copies all data into buffer
    vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});
    _motImage = new WMemoryResource("image/png");
    _motImage->setData(buffer);
    this->addResource(_motImage, "/dab/mot");
    mot_resource_path = "/dab/mot";
}

CRadioServer::~CRadioServer()
{

}

bool CRadioServer::connect(Client *client, const RadioEventCallback& handleEvent)
{
    unique_lock<recursive_mutex> lock(_mutex);

    // this prevents that the same client is logged in twice
    if (_clients.count(client) == 0)
    {
        ClientInfo clientinfo;
        
        clientinfo.sessionID = WApplication::instance()->sessionId();
        clientinfo.eventCallback = handleEvent;

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

void CRadioServer::scan_dab()
{
    _radioController->openDevice();
    _radioController->startScan();
}

void CRadioServer::scanStop()
{
    cout << "CRadioServer::scanStop()" << endl;
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

    dbo::Transaction transaction{_session};

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

        cout << "ServiceID: " << service->_serviceID << " Label: " << service->_label << " Current Channel " << service->_currentChannel << endl;
        _session.add(move(service));
    });

    //_session.flush();
}

void CRadioServer::newRadioChannel(uint32_t serviceId, string serviceName, string channelID)
{
    stringstream stream;
    stream << hex << serviceId;
    string public_serviceId = stream.str();

    cout << "Waverider::GUI::newRadioChannel" << public_serviceId << " " << serviceName << " " << channelID << endl;

    // Save this also on the Serverside to store into a database instance
    CService *newService = new CService();
    newService->_serviceID = public_serviceId;
    newService->_label = serviceName;
    newService->_currentChannel = channelID;
    _foundedChannels.push_back(newService);

    postRadioEvent(RadioEvent(serviceId, serviceName, channelID));
}

void CRadioServer::newEnsemble(string name) 
{
    cout << "... new Ensemble says the backend " << endl;
    /* WMenuItem* item = mChannelMenu->addItem(name, make_unique<WTextArea>(name));

    WApplication::instance()->triggerUpdate();*/
}

void CRadioServer::getDABChannels()
{
    cout << "CRadioServer::Get Channels from DB" << endl;
    
    try
    {
        dbo::Transaction transaction{_session};

        typedef dbo::collection< dbo::ptr<CService> > Channels;
        Channels channels = _session.find<CService>().orderBy("id");
    
        for(Channels::const_iterator it=channels.begin(); it != channels.end(); ++it)
        {
            dbo::ptr<CService> s = *it;

            cout << "Load channel: " << s->_serviceID << "; " << s->_label << "; " << s->_currentChannel << ";" << endl;

            uint32_t serviceid = static_cast<uint32_t>(stoul(s->_serviceID.c_str(), NULL, 16));

            postRadioEvent(RadioEvent(serviceid, s->_label, s->_currentChannel));
        }
    }
    catch (dbo::Exception e)
    {
        // Database not exists. Do scan.
    }
}

void CRadioServer::setChannel(uint32_t serviceid, string serviceLabel, string channel)
{
    if (_jplayerStreamer->isStreaming())
    {
        _jplayerStreamer->stopStreaming();
    }

    _jplayerStreamer->setChannel(serviceid, serviceLabel, channel);
    _jplayerStreamer->prepareStreaming();

    postRadioEvent(RadioEvent(serviceid, serviceLabel, channel, EventAction::channelChange));
}

void CRadioServer::stop()
{
    if (_jplayerStreamer->isStreaming())
    {
        _jplayerStreamer->stopStreaming();
    }
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

    _motImage = new WMemoryResource(mimetype);
    _motImage->setData(mot_file.data);
    this->addResource(_motImage, mot_resource_path);

    postRadioEvent(RadioEvent(0, mot_resource_path, "", EventAction::motChange));
}

void CRadioServer::updateSNR(int snr)
{
    double percent = 1.0*snr / 16 * 100;

    cout << "SignalStrength: "  << percent << endl;

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