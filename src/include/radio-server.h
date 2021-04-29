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
#ifndef _C_RADIO_SERVER_
#define _C_RADIO_SERVER_

#include <Wt/WServer.h>
#include <Wt/WMemoryResource.h>
#include <Wt/Dbo/backend/Sqlite3.h>
#include <Wt/Dbo/collection.h>

#include <set>
#include <map>
#include <thread>
#include <mutex>
#include <fstream>
#include <iterator>
#include <vector>
#include <random>

#include "wradio-controller.h"
#include "jplayer-streamer-resource.h"
#include "wwav-streamer-resource.h"
#include "service.h"

namespace dbo = Wt::Dbo;

using namespace std;
using namespace Wt;

enum EventAction { channelFound, channelChange, motChange, signalStrength, channelScan };

class RadioEvent
{
    public:
        RadioEvent(const uint32_t serviceId, const string label, const string currentChannel)
        : _serviceID(serviceId), _label(label), _currentChannel(currentChannel) 
        { _eventAction = EventAction::channelFound; };    
        RadioEvent(const uint32_t serviceId, const string label, const string currentChannel, EventAction e)
        : _serviceID(serviceId), _label(label), _currentChannel(currentChannel) 
        { _eventAction = e; };
        RadioEvent(const string signalStrengthImage, EventAction e)
        : _signalStrengthImage(signalStrengthImage)
        { _eventAction = e; };
        RadioEvent(const string scanChannel, const string stationCount, EventAction e)
        : _currentChannel(scanChannel), _stationCount(stationCount)
        { _eventAction = e; };

        const uint32_t getServiceID() const { return _serviceID; }
        const string getLabel() const { return _label; }
        const string getCurrentChannel() const { return _currentChannel; }
        const string getChannelScan() const { return _currentChannel; } 
        const string getSignalStrengthImage() const { return _signalStrengthImage; }
        const string getStationCount() const { return _stationCount; }
        const EventAction getAction() const { return _eventAction; }

    private:
        EventAction _eventAction;
        uint32_t _serviceID;
        string _label;
        string _currentChannel;
        string _signalStrengthImage;
        string _stationCount;
};

typedef function<void (const RadioEvent&)> RadioEventCallback;

class CRadioController; // Forward Declaration
class CJPlayerStreamerResource;

class CRadioServer : public WServer 
{
    public:
        CRadioServer(int argc, char *argv[], const std::string &wtConfigurationFile=std::string());
        ~CRadioServer();

        class Client { }; // Placeholderclass for the client

        bool connect(Client *client, const RadioEventCallback& handleEvent);
        bool disconnect(Client *client);

        void scan_dab();
        void getDABChannels();  // load all DAB+ Channels from db-file
        
        /* commands for Streaming over httpd */
        void setChannel(uint32_t serviceid, string serviceLabel, string channel);
        
        void updateMOT(mot_file_t mot_file);
        void updateSNR(int snr);
        void updateSearchingChannel(string channel, string stationCount);
        void stop();

    private:
        shared_ptr<CRadioController> _radioController; 
        CJPlayerStreamerResource* _jplayerStreamer;
        WMemoryResource* _motImage;
        string mot_resource_path;
        
        // Database connectivity
        unique_ptr<dbo::backend::Sqlite3> _dbConnector;
        dbo::Session _session;
        typedef list<CService*> FoundedChannels;
        FoundedChannels _foundedChannels;
        
        struct ClientInfo 
        {
            string sessionID;
            RadioEventCallback eventCallback;
        };
        recursive_mutex _mutex;  // for connect
        typedef map<Client*, ClientInfo> ClientMap;
        ClientMap _clients;

        void postRadioEvent(const RadioEvent& radioEvent);

        // CRadioController slots
        void newRadioChannel(uint32_t serviceId, string serviceName, string channelID);
        void newEnsemble(string name);
        void scanStop();

        friend class CRadioController;
};

#endif