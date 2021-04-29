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

CRadioController::CRadioController(CRadioServer* radioServer)
    :   WObject(),
        _audioBuffer(2 * AUDIOBUFFERSIZE)
{
    _radioServer = radioServer;

    cout << "Initialising ..." << endl;
    resetTechnicalData();

    rro.decodeTII = true;
    //rro.disableCoarseCorrector=false;

    this->switchToNextChannel().connect(this, &CRadioController::nextChannel);
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
    cout << "CRadioController: deviceRestart" << endl;
    bool isPlay = false;

    if (device)
    {
        isPlay = device->restart();
    }

    if (!isPlay)
    {
        cout << "RadioController: " << "Radio device is not ready or does not exists." << endl;
        return;
    }

    cout << "Start LabelTimer" << endl;

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
        /* device.reset(CInputFactory::GetDevice(*this, "rtl_tcp"));
        // First: We use RTL_TCP only
        CRTL_TCP_Client* RTL_TCP_Client = static_cast<CRTL_TCP_Client*>(device.get());
        RTL_TCP_Client->setServerAddress("192.168.0.189");
        RTL_TCP_Client->setPort(1234);
        RTL_TCP_Client->restart();*/
        device.reset(CInputFactory::GetDevice(*this, "rtl_sdr"));

        cout << "Sollte jetzt laufen" << endl;

        //deviceId = device->getID();
    }

    /* if (_wavResource == nullptr)
    {
        _wavResource = make_shared<CWavStreamerResource>(_audioBuffer);
    } */
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
        current_EId = 0;
        currentEnsambleLabel = "";
        currentFrequency = channels.getFrequency(Channel);

        if (currentFrequency != 0 && device) 
        {
            cout << "CRadioController: Tune to channel " << Channel << " " << currentFrequency/1e6 << " Mhz" << endl;
            device->setFrequency(currentFrequency);
            cout << "CRadioController: Reset Device Buffer" << endl;
            device->reset();    // clear Buffer
        }

        if (device) 
        {
            cout << "CRadioController: New RadioReceiver" << endl;
            radioReceiver = make_unique<RadioReceiver>(*this, *device, rro, 1);
            radioReceiver->setReceiverOptions(rro);
            cout << "CRadioController: restart(isScan) RadioReceiver" << endl;
            radioReceiver->restart(isScan);
        }
    }
}

void CRadioController::startScan() 
{
    cout << "CRadioController: " << "Start channel scan" << endl;

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

void CRadioController::onNewAudio(std::vector<int16_t>&& audioData, int sampleRate, const std::string& mode) 
{
    //cout << "CRadioController::onNewAudio: new Audio " << endl;
    _audioBuffer.putDataIntoBuffer(audioData.data(), static_cast<int32_t>(audioData.size()));

    if (audioSampleRate != sampleRate) 
    {
        audioSampleRate = sampleRate;
    }
}

void CRadioController::onRsErrors(bool uncorrectedErrors, int numCorrectedErrors) 
{
    cout << "CRadioController::onRsErrors: Not implmented yet" << endl;
}

void CRadioController::onAacErrors(int aacErrors) 
{
    cout << "CRadioController::onAacErrors: Not implmented yet" << endl;
}

void CRadioController::onNewDynamicLabel(const std::string& label)
{
    cout << "CRadioController::onNewDynamicLabel: Not implmented yet" << endl;
}
    
void CRadioController::onMOT(const mot_file_t& mot_file)
{
    //cout << "CRadioController::onMOT: Not implmented yet" << endl;
    _radioServer->updateMOT( mot_file );
}

void CRadioController::onPADLengthError(size_t announced_xpad_len, size_t xpad_len)
{
    cout << "CRadioController::onPADLengthError: Not implmented yet" << endl;
}

void CRadioController::onSNR(int snr)
{
    //cout << "CRadioController::onSNR: " << snr << endl;
    _radioServer->updateSNR(this->snr);
    if (this->snr == snr)
    {
        return;
    }
    this->snr = snr;
    
    cout << "CRadioController::onSNR: " << this->snr << endl;
    // emit
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
    //cout << "CRadioController::onSyncChange: Not implmented yet" << endl;
    bool sync = (isSync == SYNCED) ? true : false;

    if (this->isSync == sync)
    {
        //cout << "sync == isSync" << endl;
        return;
    }

    this->isSync = sync;
}

void CRadioController::onSignalPresence(bool isSignal)
{
    cout << "CRadioController::onSignalPresence: " << isSignal << endl;   
    if (this->isSignal != isSignal)
    {
        this->isSignal = true;
    }

    if (isChannelScan)
    {
        cout << "CRadioController: " << "switch to next Channel" << endl; 
        cout << "CRadioController: isSignal ist " << isSignal << endl;
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
        cout << currentText << " mit der sId " << std::hex << sId << std::dec << " auf " << currentChannel << endl;
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
    cout << "CRadioController: ID of ensemble: " << eId << endl;

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
    cout << "CRadioController::onTIIMeasurement: "
         << "TII comb " << m.comb 
         << " pattern " << m.pattern
         << " delay " << m.delay_samples
         << " = " << m.getDelayKm()
         << " with errors " << m.error << endl;
}

void CRadioController::onMessage(message_level_t level, const std::string& text, const std::string& text2) 
{
    cout << "CRadioController::onMessage: Not implmented yet" << endl;        
}

void CRadioController::onInputFailure() 
{
    //cout << "CRadioController::InputFailure: Not implmented yet" << endl;        
    //stop();
}

// Private Slots
void CRadioController::nextChannel(bool isWait) 
{
    cout << "Weiter gehts ... wait ist " << isWait << endl;
    if (isWait) // It might be a Channel, wait 10s
    {
        channelTimer.setInterval([&]() {
            channelTimerTimeout();
        }, 60000);
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
    cout << "ChannelTimer::TimeOut" << endl;
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
            cout << "Found service " << hex << service.serviceId << dec << " Name: " << label << " auf " << currentChannel << endl;

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
    cout << "CRadioController: " << " Stop Channel scan";

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

    labelTimer.stopTimer();
    //scanStopped_.emit(true);
    if (isChannelScan)
    {
        _radioServer->scanStop();
    }
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
    // emit TitleChange

    if (isChannelScan == true) 
    {
        stopScan();
    }

    deviceRestart();
    setChannel(channel, false);
    setService(service);
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
                        cout << "CRadioController: Selecting Service failed!" << endl;
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