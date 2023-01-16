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
#ifndef _WAVERIDER_GUI_H_
#define _WAVERIDER_GUI_H_

#include <Wt/WContainerWidget.h>
#include <Wt/WBorderLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WLayout.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>
#include <Wt/WMenu.h>
#include <Wt/WMenuItem.h>
#include <Wt/WCssStyleSheet.h>
#include <Wt/WLength.h>
#include <Wt/WPushButton.h>
#include <Wt/WApplication.h>
#include <Wt/WTimer.h>
#include <Wt/WFlags.h>
#include <Wt/WLink.h>
#include <Wt/WAudio.h>
#include <Wt/WMediaPlayer.h>
#include <Wt/WEnvironment.h>
#include <Wt/WImage.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WSelectionBox.h>
#include <Wt/WWebWidget.h>
#include <Wt/WProgressBar.h>

#include "wradio-controller.h"
#include "station-item.h"
#include "wwav-streamer-resource.h"
#include "radio-server.h"
#include "csettingsformview.h"
#include "cbouqueteditorview.h"
#include "crecords.h"

using namespace Wt;
using namespace std;

class RadioEvent;
class SettingsEvent;
class BouquetEvent;
class RecordFileEvent;
class CSettingsFormView;
class CBouquetEditorView;
class CRecords;

class WaveriderGUI : public WContainerWidget, public CRadioServer::Client
{
    public:
        WaveriderGUI(CRadioServer& radioServer);
        virtual ~WaveriderGUI();

        void radioEvent(const RadioEvent& event);
        void settingsEvent(const SettingsEvent& event);
        void bouquetEvent(const BouquetEditorEvent& event);
        void recordEvent(const RecordFileEvent& event);
        void errorEvent(const ErrorEvent& event);
        CRadioServer& getRadioServer() { return _radioServer; }
        void cancelSettings() { _mainStack->setCurrentIndex(0); }
        string recordPath() { return _recordPath; }

    private:
        void scan_dab();
        void scan_internet();
        void record();
        void initialiseRadioChannels();
        void handlePathChange();
        void connect();
        void removeChannelScanItem();
        void removeWebChannelScanItem();

        string _recordPath;         // For DownloadPage

        CRadioServer &_radioServer;

        WMenu *mChannelMenu;
        WMenu *mWebChannelMenu;
        WMenu *mBouquetChannelMenu;
        WText *_favoriteText;
        unique_ptr<WContainerWidget> mMainWidget;
        WContainerWidget *_DABChannelContainer;
        WContainerWidget *_InternetChannelContainer;
        WContainerWidget *_BouquetChannelContainer;
        WContainerWidget *_motrow;
        WContainerWidget *_serviceInfoContainer;
        int _progressBarIndex = 0;
        shared_ptr<CWavStreamerResource> mAudioResource;
        WStackedWidget *_mediaPlayerContainer;
        WStackedWidget *_mainStack;
        WStackedWidget *_channelContainer;
        WText *_ServiceInfo;
        WText *_ServiceInfoSmall;
        WContainerWidget *_serviceInfoSmallContainer;
        WImage *_AntennaImg;
        WText *_infoTextDirect;
        WMediaPlayer *mAudio;
        WText *_codingLabel;
        WImage *_motImg;
        WImage *_signalImg;
        WContainerWidget *_firstRow;
        WContainerWidget *_betweenIMG;
        bool _removeChannelScanItem;
        bool _removeWebChannelScanItem;
        CSettingsFormView *_settings;
        CRecords *_records;
        CBouquetEditorView *_bouqueteditor;
        WPushButton *recordButton; 
        WPushButton *_WebAndDABSwitchButton;
        bool _dabMode;
        bool _bouquetMode;
        WSelectionBox *_audioDevice;
        CStationItem *_lastSelectedRadioStation;
        int _internetCountedCountries;

        // Slots
        void playAudio();
        void stopAudio();
        void quit();

        // GUI Slots
        void actualizeChannelList();
        void setChannel(WMenuItem* item);
        void setWebChannel(WMenuItem* item);
        void loadBouquetChannels(WMenuItem* item);
        void loadBouquets();
        
        struct RADIO_STATION {
            uint32_t serviceId;
            string serviceName;
            string channelID;
        };

        map<int, RADIO_STATION> _radioStationSet;
        bool isScan = false;

        WTimer* _channelTimer;
        WTimer* _playTimer;
        WTimer* _stationTimer;
        int countTry = 1;
};

#endif