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
#include "wradio-controller.h"

#define AUDIOBUFFERSIZE 32768

CRadioController::CRadioController(CRadioServer* radioServer, CStreamingServer* streamingServer)
    :   WObject(),
        //_audioBuffer(2 * AUDIOBUFFERSIZE)
        _audioBuffer(32 * AUDIOBUFFERSIZE),
        _compressedAudioBuffer(32 * AUDIOBUFFERSIZE)
{
    _radioServer = radioServer;
    _streamingServer = streamingServer;

    _radioServer->log("info", "Initialising CRadioController.");
    resetTechnicalData();

    rro.decodeTII = true;
    isChannelScan = false;  // Bugfix 26.01.2022
    //rro.disableCoarseCorrector=false;
    _compressBeforeExport = false;

#if defined(HAVE_ALSA)
    // First for Devices, later reconfigure
    _alsaOutput = make_unique<CAlsaOutput>(2, 48000, _currentAudioDevice, this);
    
    _alsaOutput->getSounddeviceList(&_soundDevices);
#endif    

#if defined(HAVE_FFMPEG)
    //_exportAudio = make_unique<CMP3AudioCompression>(this);
    _exportFlag = false;
    //_StreamMetaDataAnalyser = make_unique<CIStreamMetaData>(this);
#endif

    this->switchToNextChannel().connect(this, &CRadioController::nextChannel);
    _recordToDir = "";

    _CoverLoader = make_unique<CCoverLoader>(this);

    _channelSwitch = false;
}

void CRadioController::resetTechnicalData() 
{
    currentChannel = "Unknown";
    current_EId = 0;
    currentEnsembleLabel = "";
    currentFrequency = 0;
    currentService = 0;
    currentStationType = "";
    currentLanguageType = "";
    currentTitle = "No Station";
    currentText = "";
    //errorMsg
    isSync = false;
    isFICCRC = false;
    isSignal = false;
    snr = 0;
    frequencyCorrection = 0;
    frequencyCorrectionPpm = NAN;
    //bitrate
    audioSampleRate = -1;
    //isDAB
    //frameErrors
    //rsErrors
    //aaErrors
}

void CRadioController::deviceRestart()
{
    _radioServer->log("info", "[CRadioController] deviceRestart");
    bool isPlay = false;

    if (device)
    {
        isPlay = device->restart();
    }

    if (!isPlay)
    {
        _radioServer->log("info", "[CRadioController] Radio device is not ready or does not exists.");
        
        return;
    }

    labelTimer.setInterval([&](){
        labelTimerTimeout();
    }, 40);
}

CDeviceID CRadioController::openDevice() 
{
    closeDevice();

    isChannelScan = false;

    if (deviceId == CDeviceID::UNKNOWN) 
    {
        if (_radioServer->getSavedDevice() == "rtl-tcp")
        {
            device.reset(CInputFactory::GetDevice(*this, "rtl_tcp"));
            CRTL_TCP_Client* RTL_TCP_Client = static_cast<CRTL_TCP_Client*>(device.get());
            RTL_TCP_Client->setServerAddress(_radioServer->getSavedIPAddress());
            RTL_TCP_Client->setPort(stoi(_radioServer->getSavedPort()));
            RTL_TCP_Client->restart();
        }
        else
        {
            device.reset(CInputFactory::GetDevice(*this, "rtl_sdr"));
        }
    }

    return device->getID();
}

void CRadioController::closeDevice()
{
    radioReceiver.reset();
    device.reset();             // Dieses reset ist eine Member-Funktion des intelligenten Zeigers!
                                // mit reset werden die Resourcen dieses Zeigers wieder freigegeben.
                                // (oder ihn per Parameter neu initialisieren)
    deviceId = CDeviceID::UNKNOWN;
    
    resetTechnicalData();
}

void CRadioController::setChannel(string Channel, bool isScan)
{
    if (currentChannel != Channel) 
    {
        currentChannel = Channel;
        _channelSwitch = true;
        current_EId = 0;
        currentEnsambleLabel = "";
        currentFrequency = channels.getFrequency(Channel);

        if (currentFrequency != 0 && device) 
        {
            _radioServer->log("info", "[CRadioController] Tune to channel " + Channel);
            //cout << "CRadioController:  " << Channel << " " << currentFrequency/1e6 << " Mhz" << endl;
            device->setFrequency(currentFrequency);
            _radioServer->log("info", "[CRadioController] Reset Device Buffer.");
            device->reset();    // clear Buffer
        }

        if (device) 
        {
            _radioServer->log("info", "[CRadioController] New RadioReceiver.");
            
            radioReceiver = make_unique<RadioReceiver>(*this, *device, rro, 1);
            radioReceiver->setReceiverOptions(rro);
            
            _radioServer->log("info", "[CRadioController] restart(isScan) RadioReceiver");
            
            radioReceiver->restart(isScan);
        }
    }
}

void CRadioController::startScan() 
{
    _radioServer->log("info", "[CRadioController] Start channel scan.");
    
    deviceRestart();

    // Start with the lowest Frequency
    string Channel = Channels::firstChannel;
    //string Channel = "5D";
    setChannel(Channel, true);
    
    isChannelScan = true;
    stationCount = 0;
    currentTitle = "Scanning " + Channel;
    currentText = "Found channels " + to_string(stationCount);

    _radioServer->updateSearchingChannel(Channel, to_string(stationCount));

    currentService = 0;
    currentStationType = "";
    currentLanguageType = "";
}

// Slots, called from the backend
void CRadioController::onFrameErrors(int frameErrors) 
{
    //cout << "CRadioController::onFrameErrors: Not implmented yet" << endl;
}

void CRadioController::onNewCompressedAudio(uint8_t* compressedData, int length, bool dabplussource)
{
    if (!dabplussource)
    {
        // this was only an idea, i did not know, that the most radio stations are using
        // icycast as a service for sending metadata.

        //_StreamMetaDataAnalyser->start_analysing(); // => if the use is wanted, then ffmpeg is needed!
        //_StreamMetaDataAnalyser->copyToAnalyseBuffer(compressedData, length);
        if (to_bool(_radioServer->_settings()->_doNotReEncodeIChannels))
        {
            if (_recordFlag)
            {
                if (!_AudioRecorder)
                {
                    string path = "";
                    string filename = "";
                    if (_recordToDir.length() > 0)
                    {
                        string codec = _radioServer->getInternetChannelCodec();
                        filename = getFilenameForRecord(codec);
                        path = _recordToDir + "/" + filename;
                    }

                    if (filename.find("[ERROR]") == string::npos)
                    {
                        _AudioRecorder = make_unique<COriginalAudioCompression>(this, path);
                        _AudioRecorder->setSampleRate(audioSampleRate);
                        _AudioRecorder->start_compression(false);
                        _AudioRecorder->directFeed((int16_t*)compressedData, length);

                        // save time
                        _radioServer->saveRecordStartTime();
                        _radioServer->saveFilename(filename);
                    }
                    else
                    {
                        // Failure occurs
                        _recordFlag = false;
                        _radioServer->errorOccur(filename);
                    }
                }
                else
                {
                    _AudioRecorder->directFeed((int16_t*)compressedData, length);
                }
            }
            else
            {
                if (_AudioRecorder)
                {
                    _AudioRecorder->stop_compression();
                    _AudioRecorder = nullptr;
                }
            }
        }
    }

    if (_exportFlag)
    {
        if (!dabplussource)
        {
            _compressedAudioBuffer.putDataIntoBuffer(compressedData, length);
            _compressBeforeExport = false;
        }
        else
        {
            _radioServer->log("info", "[CRadioController] DAB+ has to be reencoded for export ...");
            
            _compressBeforeExport = true;
            _exportFlag = false;    // Ignore further future calls till user change back to an internetchannel.
        }
    }
}

void CRadioController::reencodeandExportAudioForStreaming(int16_t* data, int len, int sampleRate)
{
#if defined(HAVE_FFMPEG)
    if (_compressBeforeExport)
    {
        if (!_Export)
        {
            _Export = make_unique<CFLACAudioCompression>(this);   // BEST QUALITY
            _Export->setSampleRate(audioSampleRate);
            _Export->start_compression(false);
            _Export->directFeed(data, len);
        }
        else
        {
            _Export->directFeed(data, len);
        }
    }
    else
    {
        if (_Export)
        {
            _Export->stop_compression();
            _Export = nullptr;
        }
    }
#endif
}

void CRadioController::saveAudioStreamAsFile(int16_t* data, int len, int sampleRate)
{
    if (_radioServer->isInternetChannelPlaying() && 
        to_bool(_radioServer->_settings()->_doNotReEncodeIChannels))
    {
        return; // Nothing here to do for us
    }

    if (_recordFlag)
    {
        if (!_AudioRecorder)
        {
            string path = "";
            string filename = "";
            if (_recordToDir.length() > 0)
            {
                filename = getFilenameForRecord(_radioServer->getEndingForRecordingFile());
                path = _recordToDir + "/" + filename;
            }

            if (filename.find("[ERROR]") == string::npos)
            {
#if defined(HAVE_FFMPEG)
                switch (_radioServer->getRecordFormat())
                {
                    case AudioCompression::AAC:
                        _AudioRecorder = make_unique<CAACAudioCompression>(this, path);
                    break;
                    case AudioCompression::ALAC:
                        _AudioRecorder = make_unique<CALACAudioCompression>(this, path);
                    break;
                    case AudioCompression::MP2:
                        _AudioRecorder = make_unique<CMP2AudioCompression>(this, path);
                    break;
                    case AudioCompression::MP3:
                        _AudioRecorder = make_unique<CMP3AudioCompression>(this, path);
                    break;
                    case AudioCompression::FLAC:
                        _AudioRecorder = make_unique<CFLACAudioCompression>(this, path);
                    break;
                    case AudioCompression::VORBIS:
                        _AudioRecorder = make_unique<CVORBISAudioCompression>(this, path);
                    break;
                    case AudioCompression::WAV:
                    default:
                        _AudioRecorder = make_unique<CNoAudioCompression>(this, path);
                    break;
                }
#else
                _AudioRecorder = make_unique<CNoAudioCompression>(this, path);
#endif
                _AudioRecorder->setSampleRate(audioSampleRate);
                _AudioRecorder->start_compression(false);
                _AudioRecorder->directFeed(data, len);

                // save time
                _radioServer->saveRecordStartTime();
                _radioServer->saveFilename(filename);
            }
            else
            {
                // Failure occurs
                _recordFlag = false;
                _radioServer->errorOccur(filename);
            }
        }
        else
        {
            _AudioRecorder->directFeed(data, len);
        }
    }
    else
    {
        if (_AudioRecorder)
        {
            _AudioRecorder->stop_compression();
            _AudioRecorder = nullptr;
        }
    }
}

void CRadioController::sendPCMToSpeakers(std::vector<int16_t>&& audioData, int sampleRate, const std::string& mode, bool reinitalsa)
{
#if defined(HAVE_ALSA)
    if (reinitalsa)
    {
        _alsaOutput->closeDevice();
        _alsaOutput = make_unique<CAlsaOutput>(2, audioSampleRate, _currentAudioDevice, this);
    }
    
    _alsaOutput->playPCM(move(audioData));
#endif
}

void CRadioController::onNewAudio(std::vector<int16_t>&& audioData, int sampleRate, const std::string& mode) 
{
    //cout << "CRadioController::onNewAudio: new Audio " << audioData.size() << endl;
    bool newAudioOutput = false;

    _audioBuffer.putDataIntoBuffer(audioData.data(), static_cast<int32_t>(audioData.size()));

    if (audioSampleRate != sampleRate && _channelSwitch) 
    {
        audioSampleRate = sampleRate;
        newAudioOutput = true;
        _channelSwitch = false;
    }

    // if wanted, stream this over a socket
    this->reencodeandExportAudioForStreaming(audioData.data(), static_cast<int32_t>(audioData.size()), sampleRate);

    // if channel was change we have to inform the server
    if (_radioServer->channelChange())
    {
        // inform the server
        _radioServer->audioDataIsAvailable();
    }

    // record the stream into a file
    this->saveAudioStreamAsFile(audioData.data(), static_cast<int32_t>(audioData.size()), sampleRate);

    // if possible, output to real speakers
    this->sendPCMToSpeakers(move(audioData), sampleRate, mode, newAudioOutput);
}

void CRadioController::setRecordToDir(string dir) 
{ 
    _recordToDir = dir; 
}

// looks for files with similar names and find the highest number. if file exists automaticly adds 
// a higher number
string CRadioController::getFilenameForRecord(string ending)
{
    string filename = "";

    int max_number = -1;

    DIR *directory;
    struct dirent *file;
    directory = opendir(_recordToDir.c_str());
    if (directory != NULL)
    {
        while (file = readdir(directory)) 
        {
            string filename_temp(file->d_name);
            if (filename_temp.find(_recordFromChannel) == 0)
            {
                // get the number
                size_t pos = filename_temp.find("_");

                if (pos != string::npos)
                {
                    string n = filename_temp.substr(pos + 1, filename_temp.find(".") - pos - 1);
                    int nr = atoi(n.c_str());
                    if (nr > max_number)
                    {
                        max_number = nr;
                    }
                }
                else
                {
                    if (max_number == -1)
                    {
                        max_number = 1;
                    }
                }
            }
        }

        if (max_number == -1)
        {
            filename = _recordFromChannel + "." + ending;
        }
        else
        {  
            stringstream ss;
            max_number++;
            ss << max_number;

            filename = _recordFromChannel + "_" + ss.str() + "." + ending;
        }
        
        return filename;
    }
    else
    {
        string ret_string = "";
        switch(errno)
        {
            case EACCES: ret_string = "[ERROR] Search permission is denied for the component of the path prefix of " + _recordToDir + " or read permission is denied for " + _recordToDir + ".";
            break;
            case ELOOP: ret_string = "[ERROR] A loop exists in symbolic links encountered during resolution of the " + _recordToDir + " argument.";
            break;
            case ENAMETOOLONG: ret_string = "[ERROR] The length of the dirname argument exceeds {PATH_MAX} or a pathname component is longer than {NAME_MAX}.";
            break;
            case ENOENT: ret_string = "[ERROR] A component of dirname does not name an existing directory or " + _recordToDir + " is an empty string.";
            break;
            case ENOTDIR: ret_string = "[ERROR] A component of dirname is not a directory.";
            break;
            default:
            ret_string = "[ERROR] An error occur, please check your filepath.";
        }

        return ret_string;
    }
}

void CRadioController::onRsErrors(bool uncorrectedErrors, int numCorrectedErrors) 
{
    _radioServer->log("info", "[CRadioController] onRsErrors: Not implmented yet ...");
}

void CRadioController::onAacErrors(int aacErrors) 
{
    _radioServer->log("info", "[CRadioController] onAacErrors: Not implmented yet ...");
}

void CRadioController::onNewDynamicLabel(const std::string& label)
{
    if (_radioServer->getRetrieveMetadata())
    {
        string Artist = _CoverLoader->getArtist(label);
        string Title = _CoverLoader->getTitle(label);

        _radioServer->updateNewTitle("Artist/Group: " + Artist + " Title: " + Title);
    }
}
    
void CRadioController::onArtistAndTitle(string title)
{
    if (_radioServer->getRetrieveMetadata())
    {
        // Called from CCoverLoader
        _radioServer->updateNewTitle(title);
    }
}

void CRadioController::onNoCoverFound()
{
    if (_radioServer->getRetrieveMetadata())
    {
        string url = _radioServer->getRadioStationFavIcon();

        if (url.length() > 0)
        {
            _CoverLoader->loadFavIcon(url);
        }
    }
}

void CRadioController::onMetaData(string metadata)
{
    if (_radioServer->getRetrieveMetadata())
    {
        _radioServer->saveRecordStartTime(); // actualize the RecordTime

        if (metadata.compare("StreamTitle='';") != 0)
        {
            _CoverLoader->searchCoverFor(metadata);
        }
    }
}

void CRadioController::onMOT(const mot_file_t& mot_file)
{
    if (_radioServer->getRetrieveMetadata())
    {
        _radioServer->updateMOT( mot_file );
        
        if (_recordFlag)
        {
            _radioServer->saveMot( mot_file );
        }
    }
}

void CRadioController::onPADLengthError(size_t announced_xpad_len, size_t xpad_len)
{
    _radioServer->log("info", "[CRadioController] onPADLengthError: Not implmented yet ...");
}

void CRadioController::onSNR(int snr)
{
    _radioServer->updateSNR(this->snr);
    if (this->snr == snr)
    {
        return;
    }
    this->snr = snr;
    
    //cout << "CRadioController::onSNR: " << this->snr << endl;
}

void CRadioController::onFrequencyCorrectorChange(int fine, int coarse) 
{
    //cout << "CRadioController::onFrequencyCorrectorChange: Not implmented yet" << endl;
    if (frequencyCorrection == coarse + fine) 
    {
        return;
    }

    frequencyCorrection = coarse + fine;

    if (currentFrequency != 0)
    {
        frequencyCorrectionPpm = -1000000.0f * static_cast<float>(frequencyCorrection) / static_cast<float>(currentFrequency);
    }
    else 
    {
        frequencyCorrection = NAN;
    }
}

void CRadioController::onSyncChange(char isSync)
{
    bool sync = (isSync == SYNCED) ? true : false;

    if (this->isSync == sync)
    {
        return;
    }

    this->isSync = sync;
}

void CRadioController::onSignalPresence(bool isSignal)
{
    if (this->isSignal != isSignal)
    {
        this->isSignal = true;
    }

    if (isChannelScan)
    {
        nextChannel_.emit(isSignal);
    }
}

void CRadioController::onServiceDetected(uint32_t sId)
{
    //cout << "CRadioController::onServiceDetected: Not implmented yet" << endl;   
    if (isChannelScan == true)
    {
        stationCount++;
        currentText = "Found channels: " + to_string(stationCount);
        
        _radioServer->log("info", "[CRadioController] " + currentText + " auf " + currentChannel + ".");
        
        //cout << currentText << " mit der sId " << std::hex << sId << std::dec << " auf " << currentChannel << endl;
        // emit Textchanged für Wt
        _radioServer->updateSearchingChannel(currentChannel, to_string(stationCount));
    }

    if (sId <= 0xFFFF) 
    {
        // Exclude Data Services from the List
        pendingLabels.push_back(sId);
    }
}
    
void CRadioController::onNewEnsemble(uint16_t eId)
{
    //cout << "CRadioController::onNewEnsemble: Not implmented yet" << endl;
    //[ToDo] emitten, dass was neues gefunden wurde
    _radioServer->log("info", "[CRadioController] ID of ensemble: " + to_string(eId));

    if (current_EId == eId)
    {
        return;
    }

    current_EId = eId;
}

void CRadioController::onSetEnsembleLabel(DabLabel& label)
{
    //cout << "CRadioController::onSetEnsembleLabel: Not implmented yet" << endl;   
    if (currentEnsambleLabel == label.utf8_label()) 
    {
        return;
    }

    currentEnsambleLabel = label.utf8_label();
    cout << "RadioController: Label of Ensemble: " << currentEnsambleLabel << endl;

    // Label Change Emitten 
    newEnsemble_.emit(currentEnsambleLabel);
}

void CRadioController::onDateTimeUpdate(const dab_date_time_t& dateTime)
{
    //cout << "CRadioController::onDateTimeUpdate: Not implmented yet" << endl;   
    //[ToDo] emit der Zeit und des Datums
    WDate date;
    WTime time;

    date.setDate(dateTime.year, dateTime.month, dateTime.day);
    currentDateTime.setDate(date);
    time.setHMS(dateTime.hour, dateTime.minutes, dateTime.seconds);
    currentDateTime.setTime(time);

    int OffsetFromUtc = dateTime.hourOffset * 3600 + dateTime.minuteOffset * 60;
    // ToDo: Für später -> Wt unterstützt keine Offset
    // dateTimeChanged emitten
}

void CRadioController::onFIBDecodeSuccess(bool crcCheckOk, const uint8_t* fib)
{
    //cout << "CRadioController::onFIBDecodeSuccess: Not implmented yet" << endl;      
    (void)fib;
    if (isFICCRC == crcCheckOk)
    {
        return;
    }

    isFICCRC = crcCheckOk;
}

void CRadioController::onNewImpulseResponse(std::vector<float>&& data)
{
    //cout << "CRadioController::onNewImpulseResponse: Not implmented yet" << endl;        
    lock_guard<mutex> lock(impulseResponseBufferMutex);
    impulseResponseBuffer = move(data);
}

void CRadioController::onConstellationPoints(std::vector<DSPCOMPLEX>&& data)
{
    //cout << "CRadioController::onConstellationPoints: Not implmented yet" << endl;        
    lock_guard<mutex> lock(constellationPointBufferMutex);
    constellationPointBuffer = move(data);
}

void CRadioController::onNewNullSymbol(std::vector<DSPCOMPLEX>&& data)
{
    //cout << "CRadioController::onNewNullSymbol: Not implmented yet" << endl;  
    lock_guard<mutex> lock(nullSymbolBufferMutex);
    nullSymbolBuffer = move(data);      
}

void CRadioController::onTIIMeasurement(tii_measurement_t&& m)
{
    _radioServer->log("debug", "[CRadioController] onTIIMeasurement: TII comb " + 
        to_string(m.comb) + " pattern " + to_string(m.pattern) +
        " delay " + to_string(m.delay_samples) + 
        " = " + to_string(m.getDelayKm()) + " with errors " + to_string(m.error));
}

void CRadioController::onMessage(message_level_t level, const std::string& text, const std::string& text2) 
{
    // Error Messages!! [ToDo: implement them!!!]
    _radioServer->log("debug", "[CRadioController] onMessage: " + text + " / " + text2);
}

void CRadioController::onInputFailure() 
{
    _radioServer->log("debug", "[CRadioController] InputFailure: Not implmented yet ... ");
}

// Private Slots
void CRadioController::nextChannel(bool isWait) 
{
    //cout << "Weiter gehts ... wait ist " << isWait << endl;
    if (isWait) // It might be a Channel, wait 10s
    {
        channelTimer.setInterval([&]() {
            channelTimerTimeout();
        }, 30000);
    }
    else
    {
        auto Channel = channels.getNextChannel();

        if (!Channel.empty()) 
        {
            setChannel(Channel, true);
            int index = channels.getCurrentIndex() + 1;

            currentTitle = "Scanning " + Channel;
            currentText = "Found channels " + to_string(stationCount);

            //scanProgress emitten hier => und index übergeben
            _radioServer->updateSearchingChannel(Channel, to_string(stationCount));
        }
        else
        {
            _radioServer->updateSearchingChannel("Channel scan finished", to_string(stationCount));
            stopScan();
        }
    }
}

void CRadioController::channelTimerTimeout() 
{
    _radioServer->log("debug", "[CRadioController] ChannelTimer -> TimeOut");
    channelTimer.stopTimer();

    if (isChannelScan) 
    {
        nextChannel(false);
    }
}

/**
 *  During scan, only labels would be used.
 */
void CRadioController::labelTimerTimeout()
{
    //cout << "LabelTimer::TimeOut" << endl;

    if (radioReceiver and not pendingLabels.empty())
    {
        const auto sId = pendingLabels.front();
        pendingLabels.pop_front();

        string label;

        auto service = radioReceiver->getService(sId);
        if (service.serviceId != 0) 
        {
            label = service.serviceLabel.utf8_label();
        }

        if (not label.empty())
        {
            // emit StationNameReceived
            _radioServer->log("debug", "Found service " + to_string(service.serviceId) + " Name: " + label + " auf " + currentChannel);
            //cout << "Found service " << hex << service.serviceId << dec << " Name: " << label << " auf " << currentChannel << endl;

            //nextService_.emit(service.serviceId, label, currentChannel);
            if (isChannelScan)
            {
                _radioServer->newRadioChannel(service.serviceId, label, currentChannel);
            } 

            if (currentService == sId) 
            {
                currentTitle = label;
                //emit TitleChanged
            }
        }
        else
        {
            pendingLabels.push_back(sId);
        }
    }
}

void CRadioController::stopScan() 
{
    _radioServer->log("debug", "[CRadioController] Stop channel scan.");
    
    currentTitle = "No Station";
    currentText = "";
    currentChannel = "";

    stop();

    isChannelScan = false;
}

void CRadioController::stop()
{
    if (device) 
    {
        device->stop();
    }

    //radioReceiver->stop();
    currentTitle = "No Station";
    currentText = "";
    currentChannel = "";
    _playFlag = false;

    labelTimer.stopTimer();
    //scanStopped_.emit(true);
    if (isChannelScan)
    {
        _radioServer->scanStop();
    }

#if defined(HAVE_ALSA)
    // Add in vacation time in Switzerland ;-) ... this stopps the Thread in ALSA Output class
    _radioServer->log("info", "[CRadioController] Stopping sound ...");
    
    if (_alsaOutput->isRunning())
        _alsaOutput->stop();
#endif
}

void CRadioController::webStop()
{
    _playFlag = false;          // Bugfix 26.01.2022

    // First of all -> stop the recording directly
    if (_recordFlag)
    {
        _recordFlag = false;
        if (_AudioRecorder)
        {
            _AudioRecorder->stop_compression();
            _AudioRecorder = nullptr;
        }
    }

    _streamingServer->stop();

    //_StreamMetaDataAnalyser->stop_analysing();

#if defined(HAVE_ALSA)
    // Add in vacation time in Switzerland ;-) ... this stopps the Thread in ALSA Output class
    // untested. In Switzerland my iPhone did not work properly as a Tethering Device
    cerr << "Stopping sound ..." << endl;
    if (_alsaOutput->isRunning())
        _alsaOutput->stop();
#endif
}

/*****************************************************************************************
 * The following implementations are for playing a radio programme in an ensemble.
 *****************************************************************************************/

void CRadioController::play(string channel, string title, uint32_t service)
{
    if (channel == "")
    {
        return;
    }

    currentTitle = title;
    _playFlag = true;
    // emit TitleChange

    if (isChannelScan == true) 
    {
        stopScan();
    }

    deviceRestart();
    setChannel(channel, false);
    setService(service);
}

int CRadioController::currentWebSampleRate()
{
    return _streamingServer->getAudioSamplerate();
}

void CRadioController::play(string channel, string url)
{
    _radioServer->log("info", "[CRadioController] Playing channel " + channel + " with url " + url);
    
    if (channel == "")
    {
        return;
    }

    _playFlag = true;
    
    if (isChannelScan == true) 
    {
        _radioServer->log("info", "[CRadioController] Channel Scan is true?, this is redicoulous ...");
        
        stopScan();
    }

    _streamingServer->setInternetChannel(url);
}

void CRadioController::setService(uint32_t service, bool force)
{
    if (currentService != service || force)
    {
        currentService = service;
        //emit StationChanged

        stationTimer.setInterval([&]() {
            stationTimerTimeout();
        }, 1000);

        currentStationType = "";
        currentLanguageType = "";
        currentText = "";
    }
}

void CRadioController::stationTimerTimeout()
{
    if (!radioReceiver)
    {
        return;
    }

    const auto services = radioReceiver->getServiceList();
    for (const auto& s: services)
    {
        if (s.serviceId == currentService)
        {
            const auto comps = radioReceiver->getComponents(s);
            for (const auto& sc: comps) 
            {
                if (sc.transportMode() == TransportMode::Audio)
                {
                    const auto& subch = radioReceiver->getSubchannel(sc);
                    if (!subch.valid())
                    {
                        return;
                    }

                    stationTimer.stopTimer();

                    bool success = radioReceiver->playSingleProgramme(*this, "", s);
                    if (!success)
                    {
                        _radioServer->log("error", "[CRadioController] Selecting Service failed!");
                    }
                    else
                    {
                        // Metadata and Play
                        currentStationType = DABConstants::getProgramTypeName(s.programType);
                        //emit StationType
                        currentLanguageType = DABConstants::getLanguageName(s.language);
                        //emit LanguageType
                        bitRate = subch.bitrate();
                    }

                    return;
                }
            }
        }
    }
}

/* RingBuffer<uint8_t>& CRadioController::getMP3AudioRingBuffer()
{ 
    return _exportAudio->getExportBuffer(); 
}*/

void CRadioController::setExportFlag(bool flag) 
{ 
    _exportFlag = flag;
    _compressBeforeExport = flag; 
}

#if defined(HAVE_ALSA)
void CRadioController::postFoundedSoundCards()
{
    if (!_radioServer->isPlaying())
    {
        for (string name: _soundDevices)
        {
            _radioServer->soundcardFound(name);
        }
    }
}
#endif