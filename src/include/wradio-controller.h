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
#ifndef _C_RADIO_CONTROLLER_H_
#define _C_RADIO_CONTROLLER_H_

#include <Wt/WObject.h>
#include <Wt/WSignal.h>
#include <Wt/WDate.h>
#include <Wt/WTime.h>
#include <Wt/WDateTime.h>

#include <dirent.h>
#include <cstring>

#include "dab-constants.h"
#include "radio-receiver.h"
#include "ringbuffer.h"
#include "channels.h"
#include "input/input_factory.h"
#include "input/rtl_tcp.h"

#include "simple-timer.h"
#include "radio-server.h"
extern "C" 
{
    #include "wavfile.h"
}

using namespace Wt;
using namespace std;

class CRadioServer;     // Forward Declaration

class CRadioController 
    :   public WObject,
        public RadioControllerInterface,
        public ProgrammeHandlerInterface
{
    public:
        CRadioController(CRadioServer* radioServer);

        CDeviceID openDevice();
        void setChannel(string Channel, bool isScan);
        void startScan();
        void stopScan();
        void stop();
        void play(string channel, string title, uint32_t service);
        int currentSampleRate() { return audioSampleRate; }

        RingBuffer<int16_t>& getAudioRingBuffer() { return _audioBuffer; }

        // slots called From the Backend
        virtual void onFrameErrors(int frameErrors) override;
        virtual void onNewAudio(std::vector<int16_t>&& audioData, int sampleRate, const std::string& mode) override;
        virtual void onRsErrors(bool uncorrectedErrors, int numCorrectedErrors) override;
        virtual void onAacErrors(int aacErrors) override;
        virtual void onNewDynamicLabel(const std::string& label) override;
        virtual void onMOT(const mot_file_t& mot_file) override;
        virtual void onPADLengthError(size_t announced_xpad_len, size_t xpad_len) override;
        virtual void onSNR(int snr) override;
        virtual void onFrequencyCorrectorChange(int fine, int coarse) override;
        virtual void onSyncChange(char isSync) override;
        virtual void onSignalPresence(bool isSignal) override;
        virtual void onServiceDetected(uint32_t sId) override;
        virtual void onNewEnsemble(uint16_t eId) override;
        virtual void onSetEnsembleLabel(DabLabel& label) override;
        virtual void onDateTimeUpdate(const dab_date_time_t& dateTime) override;
        virtual void onFIBDecodeSuccess(bool crcCheckOk, const uint8_t* fib) override;
        virtual void onNewImpulseResponse(std::vector<float>&& data) override;
        virtual void onConstellationPoints(std::vector<DSPCOMPLEX>&& data) override;
        virtual void onNewNullSymbol(std::vector<DSPCOMPLEX>&& data) override;
        virtual void onTIIMeasurement(tii_measurement_t&& m) override;
        virtual void onMessage(message_level_t level, const std::string& text, const std::string& text2 = std::string()) override;
        virtual void onInputFailure(void) override;

        // Public Signals
        Signal<uint32_t, string, string>& newServiceFound() { return nextService_; }
        Signal<uint32_t, string, string> nextService_; 
        Signal<string>& newEnsembleFound() { return newEnsemble_; }
        Signal<string> newEnsemble_; 
        Signal<bool>& scanStopped() { return scanStopped_; }
        Signal<bool> scanStopped_; 

        // Recording
        void setRecordFlag(bool f) { _recordFlag = f; }
        bool recordFlag() { return _recordFlag; }
        bool playFlag() { return _playFlag; }
        void setRecordFromChannel(string channelName) { _recordFromChannel = channelName; }
        void setRecordToDir(string dir) { _recordToDir = dir; }

    private:
        void resetTechnicalData();
        void deviceRestart();
        void closeDevice();
        void setService(uint32_t service, bool force = false);

        CRadioServer* _radioServer;

        // Signal accessors
        Signal<bool>& switchToNextChannel() { return nextChannel_; }
        Signal<bool> nextChannel_;

        // private Slots
        void nextChannel(bool isWait);
        void channelTimerTimeout();
        void labelTimerTimeout();
        void stationTimerTimeout();
        
        shared_ptr<CVirtualInput> device;
        unique_ptr<RadioReceiver> radioReceiver;
        RadioReceiverOptions rro;
        string currentChannel;
        uint16_t current_EId;
        int32_t currentFrequency;
        uint32_t currentService;
        string currentEnsambleLabel;
        Channels channels;
        bool isChannelScan;
        int stationCount = 0;
        string currentTitle;
        string currentText;
        string currentStationType;
        string currentLanguageType;
        bool isSync;
        int frequencyCorrection = 0;
        float frequencyCorrectionPpm = 0.0;
        bool isSignal;
        mutex impulseResponseBufferMutex;
        vector<float> impulseResponseBuffer;
        bool isFICCRC;
        mutex constellationPointBufferMutex;
        vector<DSPCOMPLEX> constellationPointBuffer;
        mutex nullSymbolBufferMutex;
        vector<DSPCOMPLEX> nullSymbolBuffer;
        int snr = 0;
        list<uint32_t> pendingLabels;
        WDateTime currentDateTime;
        string currentEnsembleLabel;
        int bitRate;
        CDeviceID deviceId = CDeviceID::UNKNOWN;

        // Streaming
        RingBuffer<int16_t> _audioBuffer;
        int audioSampleRate = 0;
        
        // Timer
        CSimpleTimer channelTimer;
        CSimpleTimer stationTimer;
        CSimpleTimer labelTimer;

        // Scanning
        thread scanHandle;

        // Recording
        string getFilenameForRecord();

        bool _recordFlag = false;
        bool _playFlag = false;
        string _recordFromChannel;
        string _recordToDir;
        FILE *_fd = nullptr;

        friend class CRadioServer;
};

#endif