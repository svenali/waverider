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
#include <Wt/WIOService.h>
#include <Wt/WMemoryResource.h>
#include <Wt/Dbo/backend/Sqlite3.h>
#include <Wt/Dbo/collection.h>
#include <Wt/WLogger.h>

#include <set>
#include <map>
#include <stack>
#include <thread>
#include <mutex>
#include <fstream>
#include <iterator>
#include <vector>
#include <random>
#include <iomanip>

#include "wradio-controller.h"
#include "jplayer-streamer-resource.h"
#include "cexportaudioresource.h"
#include "wwav-streamer-resource.h"
#include "service.h"
#include "csettings.h"
#include "cstreamingserver.h"
#include "cradiobrowser.h"
#include "cdbocountry.h"
#include "cdborecord.h"

#include "tools.h"
#include "tool.h"       // Only for to_bool and so on ...

namespace dbo = Wt::Dbo;

using namespace std;
using namespace Wt;

enum EventAction { channelFound, webChannelFound, bouquetFound, channelChange, motChange, signalStrength, channelScan, soundcardFound, receiveAudio, createNextLink, createPrevLink, removeNextLink, removePrevLink, internetCountryCount, internetCountryScan, internetScanFinished, backToBouquets, newTitle };
enum SettingsAction { newSettings, updatedSettings, loadedSettings };
enum BouquetEditorEventAction { savedChannels, savedCountries, savedExactCountryCodes, savedCodecs, savedLanguages, savedTags, savedDABChannels, bouquetsLoaded, newBouquets };
enum InternetChannelScroll { nothing, prev, forward }; // skipping next 100 channel

enum CurrentPlayingStationType { noservice, dab, web }; // if stop over the Web Frontend does not work
enum ErrorAction { fileSystemError };

class SettingsEvent
{
    public:
        SettingsEvent(const string device, const string ipaddress, const string port, const string recordPath, const string radioBrowserURL, const string donotreencode, string streaming, string streamingAddress, string streamingFormat, string recordFormat, string metadata, string saveMetadata, SettingsAction e)
        : _device(device), _ipaddress(ipaddress), _port(port), _recordPath(recordPath), _radioBrowserURL(radioBrowserURL), _donotreencode(donotreencode), _streaming(streaming), _streamingAddress(streamingAddress), _streamingFormat(streamingFormat), _recordFormat(recordFormat), _metadata(metadata), _saveMetadata(saveMetadata)
        { _settingsAction = e; };
        SettingsEvent(const string recordPath, SettingsAction e)
        : _device(""), _ipaddress(""), _port(""), _recordPath(recordPath), _donotreencode("true"), _streamingAddress(""), _streamingFormat("FLAC"), _recordFormat("FLAC")
        { _settingsAction = e; };

        const string getDevice() const { return _device; }
        const string getIPAddress() const { return _ipaddress; }
        const string getPort() const { return _port; }
        const string getRecordPath() const { return _recordPath; }
        const string getRadioBrowserURL() const { return _radioBrowserURL; }
        const string getDoNotReEncode() const { return _donotreencode; } 
        const string getStreaming() const { return _streaming; }
        const string getStreamingAddress() const { return _streamingAddress; }
        const string getStreamingFormat() const { return _streamingFormat; }
        const string getRecordFormat() const { return _recordFormat; }
        const string getRetrieveMetadata() const { return _metadata; }
        const string getSaveMetadata() const { return _saveMetadata; }
        const SettingsAction getAction() const { return _settingsAction; }

    private:
        SettingsAction _settingsAction;
        string _device;
        string _ipaddress;
        string _port;
        string _recordPath;
        string _radioBrowserURL;
        string _donotreencode;
        string _streaming;
        string _streamingAddress;
        string _streamingFormat;
        string _recordFormat;
        string _metadata;
        string _saveMetadata;
};

class RadioEvent
{
    public:
        RadioEvent(const uint32_t serviceId, const string label, const string currentChannel)
        : _serviceID(serviceId), _label(label), _currentChannel(currentChannel) 
        { _eventAction = EventAction::channelFound; };    
        RadioEvent(const string label, const string url)
        :  _label(label), _url(url) 
        { _eventAction = EventAction::webChannelFound; }; 
        RadioEvent(const uint32_t serviceId, const string label, const string currentChannel, EventAction e)
        : _serviceID(serviceId), _label(label), _currentChannel(currentChannel) 
        { _eventAction = e; };
        RadioEvent(const string identifier, EventAction e)
        { 
            _eventAction = e;
            if (_eventAction == EventAction::signalStrength) 
            {
                _signalStrengthImage = identifier;
            }
            else if (_eventAction == EventAction::newTitle)
            {
                _newTitle = identifier;
            }
            else if (_eventAction == EventAction::bouquetFound)
            {
                _signalStrengthImage = identifier;
            }
        };
        RadioEvent(const string scanChannel, const string stationCount, EventAction e)
        : _currentChannel(scanChannel), _stationCount(stationCount)
        { _eventAction = e; };
        RadioEvent(const int devNr, const string soundCardName, EventAction e)
        : _soundDevice(soundCardName), _soundDeviceNr(devNr)
        { _eventAction = e; };
        RadioEvent(int number, string country, string countryCode, EventAction e)
        : _countryScan(number), _countryCount(number), _currentChannel(countryCode), _label(country)
        { _eventAction = e; };
        // generic Event, if Audio is comming
        RadioEvent(EventAction e)
        { _eventAction = e; };

        const uint32_t getServiceID() const { return _serviceID; }
        const string getLabel() const { return _label; }
        const string getCurrentChannel() const { return _currentChannel; }
        const string getChannelScan() const { return _currentChannel; } 
        const string getSignalStrengthImage() const { return _signalStrengthImage; }
        const string getStationCount() const { return _stationCount; }
        const string getWebURL() const { return _url; }
        const string getSoundDevice() const { return _soundDevice; }
        const string getNewTitle() const { return _newTitle; }
        const EventAction getAction() const { return _eventAction; }
        const int getCountryCount() const { return _countryCount; }
        const int getCountryScan() const { return _countryScan; }

    private:
        EventAction _eventAction;
        uint32_t _serviceID;
        string _label;
        string _currentChannel;
        string _signalStrengthImage;
        string _stationCount;
        string _url;
        string _newTitle;
        int _countryCount;
        int _countryScan;
        string _soundDevice;
        int _soundDeviceNr; // without meaning -> useless because of overload
};

class BouquetEditorEvent
{
    public:
        BouquetEditorEvent(const vector<string> result, BouquetEditorEventAction e)
        :   _result(result) 
        { _eventAction = e; };
        BouquetEditorEvent(const map<string, string> result, BouquetEditorEventAction e)
        :   _channelList(result)
        { _eventAction = e; };
        BouquetEditorEvent(const map<string, map<string, string>> result, BouquetEditorEventAction e)
        :   _bouquets(result)
        { _eventAction = e; };
        BouquetEditorEvent(BouquetEditorEventAction e)
        { _eventAction = e; };

        const vector<string> getResult() const { return _result; };
        const map<string, string> getChannelList() const { return _channelList; };
        const BouquetEditorEventAction getAction() const { return _eventAction; };
        const map<string, map<string, string>> getBouquets() const { return _bouquets; };

    private:
        BouquetEditorEventAction _eventAction;
        vector<string> _result;
        map<string, string> _channelList;
        map<string, map<string, string>> _bouquets;
};

class RecordFileEvent
{
    public:
        RecordFileEvent(const vector<vector<string>> result)
        :_result(result) 
        {};
    
        const vector<vector<string>> getResult() const { return _result; };

    private:
        vector<vector<string>> _result;
};

class ErrorEvent
{
    public:
        ErrorEvent(string error): _message(error) {};

        const string getMessage() const { return _message; };

    private:
        string _message;
};

typedef function<void (const RadioEvent&)> RadioEventCallback;
typedef function<void (const SettingsEvent&)> SettingEventCallback;
typedef function<void (const BouquetEditorEvent&)> BouquetEditorEventCallback;
typedef function<void (const RecordFileEvent&)> RecordFileEventCallback;
typedef function<void (const ErrorEvent&)> ErrorEventCallback;

// Forward Declaration
class CRadioController;
class CJPlayerStreamerResource;
class CExportAudioResource;
class CStreamingServer;
class CRadioBrowser;
class CDBOCountry;
class CDBORecord;

class CRadioServer : public WServer 
{
    public:
        CRadioServer(int argc, char *argv[], const std::string &wtConfigurationFile=std::string(),CStreamingServer *ss=NULL);
        ~CRadioServer();

        class Client { }; // Placeholderclass for the client

        bool connect(Client *client, const RadioEventCallback& handleEvent, const SettingEventCallback& SettingEventCallback, const BouquetEditorEventCallback& bouquetEventCallback, const RecordFileEventCallback& recordFileEventCallback, const ErrorEventCallback& errorEventCallback);
        bool disconnect(Client *client);

        void scan_dab();
        void scan_internet();
        void scan_internet_finished();
        void recordStream();
        void reactivateRecordingChannel(); // If Record Session is still open...
        bool isRecording();
        bool isPlaying();
        bool isInternetChannelPlaying();
        void getDABChannels();  // load all DAB+ Channels from db-file
        void getSettings();     // load Settings
        void saveSettings(string device, string ipaddress, string port, string recordPath, string radioBrowserURL, string donotreencode, string streaming, string streamingAddress, string streamingFormat, string recordFormat, string metadata, string saveMetadata);
        
        /* commands for Streaming over httpd */
        void setChannel(uint32_t serviceid, string serviceLabel, string channel);
        void setWebChannel(string serviceLabel, string url);
        void audioDataIsAvailable();
        
        void updateMOT(mot_file_t mot_file);
        void updateSNR(int snr);
        void updateSearchingChannel(string channel, string stationCount);
        void stop();
        void webStop();
        void shutup();

        // sends Metadata to client
        void updateNewTitle(string newTitle);
        string getRadioStationFavIcon();

        // Settings
        string getSavedDevice() { return _settings()->_radioDevice; };
        string getSavedIPAddress() { return _settings()->_ipaddress; };
        string getSavedPort() { return _settings()->_port; };
        bool getDoNotReencodeInternetChannels();
        AudioCompression getRecordFormat();
        string getEndingForRecordingFile();
        bool getStreaming();
        string getStreamingAddress();
        AudioCompression getStreamingFormat();
        bool getRetrieveMetadata();
        bool getSaveMetadata();
        string getInternetChannelCodec();

        /* Bouquet-Editor */
        void getSavedDABChannels();
        void getAllWebChannels();
        void getSavedCountries();
        void getSavedCountryCodeExact();
        void getSavedCodecs();
        void getSavedLanguages();
        void getSavedTags();
        void getSavedChannelsOrderByVotes();
        void getWebChannelsFromCountry(string fromCountry);
        void getWebChannelsFromCountryCodeExact(string fromCode);
        void getWebChannelsFromCodec(string codec);
        void getWebChannelsFromLanguage(string language);
        void getWebChannelsFromTags(string tag);
        void saveBouquetsInDB(map<string, map<string,string>> bouquets);
        void getBouquetsInDB();

        /* Internet Streaming - Internet Radio */
        void setInternetChannel(string link);
        CRadioController* getRadioController() { return _radioController.get(); };
        void addWebChannel(string name, string url); 
        void getWebChannels(InternetChannelScroll action);  // load all Web Channels from db-file
        bool channelChange() { return _channelChange; };
        void addCountry(string code);
        dbo::ptr<CDBOCountry> getCountryPtr(string code);
        void addWebChannel(string stationuuid, string name, string url, string url_resolved, string homepage, string favicon, string tags, string country, string countrycode, string state, string language, string languagecode, int votes, string codec, int bitrate);

        /* Boquets */
        void getBouquets();
        void loadChannelsInBouquet(string bouquetName);

        void tellCountryCount(int countedCountries);
        void tellCountryScan(int numberOfCountry, string country, string countryCode);

        // Sound Events
        void soundcardFound(string name);

        /* Recorded Files */
        void showContentOfRecordedFile(string filename);

        /* overwrites the supermethod and collect the ressources in a vector */
        void addResource(std::shared_ptr<WResource> r, const string& path);
        /* for instance: deleteRessources("/recordedFile") */
        void deleteResources(string mainPath);

        /* Errors */
        void errorOccur(string message);

        /* Logging */
        void setLogger(WLogger& logger);
        void log(string type, string message);
        
    private:
        WLogger _logger;
        CurrentPlayingStationType _currentPlayingStationType;

        string generateUUID();

        CStreamingServer* _streamingServer;
        unique_ptr<CRadioController> _radioController; 
        shared_ptr<CJPlayerStreamerResource> _jplayerStreamer;
        shared_ptr<CExportAudioResource> _exportAudioResource;
        shared_ptr<WMemoryResource> _motImage;
        string mot_resource_path;
        string _playingChannel;     // For Records
        string _playingURL;         // For NoCoverFound Emits
        bool _channelChange;        // If channel was fresh changed true, otherwise false
        mot_file_t _lastMotFile;    // if no one new is coming, take the last
        unique_ptr<CRadioBrowser> _radioBrowser;
        //int _lastWebIDFromDB;
        std::stack<int> _lastWebIDFromDBs;

        // Database connectivity
        unique_ptr<dbo::backend::Sqlite3> _dbConnector;
        dbo::Session _session;
        //typedef list<CService*> FoundedChannels;
        typedef list<dbo::ptr<CService>> FoundedChannels;
        FoundedChannels _foundedChannels;
        typedef vector<dbo::ptr<CDBOCountry>> FoundedCountries;
        FoundedCountries _foundedCountries;

        // Informations about records
        chrono::system_clock::time_point _TimePointRecordStart;
        chrono::system_clock::duration _DurationTimePoint;
        string _Songname;
        string _Artist;
        string _RecordFilename;
        void saveRecordStartTime();
        void saveArtistAndTitle(string title);
        void saveFilename(string fn);
        void saveTimeOfRecordStart();
        void saveMot(mot_file_t mot_file);
        
        struct ClientInfo 
        {
            string sessionID;
            RadioEventCallback eventCallback;
            SettingEventCallback settingsCallback;
            BouquetEditorEventCallback bouquetCallback;
            RecordFileEventCallback fileCallback;
            ErrorEventCallback errorCallback;
        };
        recursive_mutex _mutex;  // for connect
        typedef map<Client*, ClientInfo> ClientMap;
        ClientMap _clients;

        // Settings
        bool _refreshSettings;
        dbo::ptr<CSettings> _readedSettings;
        dbo::ptr<CSettings> _settings();
        
        void postRadioEvent(const RadioEvent& radioEvent);
        void postSettingEvent(const SettingsEvent& settingEvent);
        void postBouquetEditorEvent(const BouquetEditorEvent& bouquetEditorEvent);
        void postRecordFileEvent(const RecordFileEvent& recordFileEvent);
        void postErrorEvent(const ErrorEvent& errorEvent);

        // CRadioController slots
        void newRadioChannel(uint32_t serviceId, string serviceName, string channelID);
        void newEnsemble(string name);
        void scanStop();

        void initDatabase();
        void readSettings();

        // Ressources / Entry Points
        vector<string> _entryPoints;

        friend class CRadioController;
};

#endif