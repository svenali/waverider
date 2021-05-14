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
#include "waverider-gui.h"
#include "station-item.h"

WaveriderGUI::WaveriderGUI(CRadioServer& radioServer)
    :   WContainerWidget(),
        _radioServer(radioServer)
{
    WApplication *app = WApplication::instance();
    cout << "APP ID: " << app->id() << endl;

    _removeChannelScanItem = true;

    this->setStyleClass("container-fluid");
    auto header_row = make_unique<WContainerWidget>();
    header_row->setStyleClass("row");

    /** Title **/
    auto settingsButton = make_unique<WPushButton>();
    settingsButton->setIcon("resources/icons/gear-wheel-32x32.png");
    settingsButton->setStyleClass("north-widget col-xs-1 col-md-1 col-lg-1");

    auto fileDownloadButton = make_unique<WPushButton>();
    fileDownloadButton->setIcon("resources/icons/folder-32x32.png");
    fileDownloadButton->setStyleClass("north-widget col-xs-1 col-md-1 col-lg-1");
    
    unique_ptr<WText> title = make_unique<WText>("<h2>Waverider</h2>");
    //title->setStyleClass("h1");
    title->setTextAlignment(AlignmentFlag::Center);
    title->setStyleClass("north-widget col-xs-10 col-md-10 col-lg-10");
    header_row->addWidget(move(title));
    
    auto row_middle = make_unique<WContainerWidget>();
    row_middle->setStyleClass("row");
    
    _ChannelContainer = row_middle->addNew<WContainerWidget>();
    _ChannelContainer->setStyleClass("west-widget col-xs-12 col-md-3 col-lg-3");
    
    mMainWidget = make_unique<WContainerWidget>();
    mMainWidget->setStyleClass("east-widget col-xs-12 col-md-9 col-lg-9");

    /*****************************************************************************************************
     *   Settings and Services
     *****************************************************************************************************/
    _mainStack = mMainWidget->addNew<WStackedWidget>();
    settingsButton->clicked().connect([=] 
    {
        if (_mainStack->currentIndex() == 0 || _mainStack->currentIndex() == 2)
        {
            _mainStack->setCurrentIndex(1);
        }
        else
        {
            _mainStack->setCurrentIndex(0);
        }
    });
    fileDownloadButton->clicked().connect([=]
    {
        if (_mainStack->currentIndex() == 0 || _mainStack->currentIndex() == 1)
        {
            _mainStack->setCurrentIndex(2);
        }
        else
        {
            _mainStack->setCurrentIndex(0);
        }
    });
    header_row->addWidget(move(fileDownloadButton));
    header_row->addWidget(move(settingsButton));
    this->addWidget(move(header_row));

    /*****************************************************************************************************
     *   Service Info
     *****************************************************************************************************/
    //auto mMain_row = mMainWidget->addNew<WContainerWidget>();
    auto mMain_row = _mainStack->addNew<WContainerWidget>();
    mMain_row->setStyleClass("row");

    auto infoServiceWidget = mMain_row->addNew<WContainerWidget>();
    infoServiceWidget->setStyleClass("info-service-widget col-xs-12 col-md-12 col-lg-12");

    unique_ptr<WText> serviceTitle = make_unique<WText>("<h5>Service Overview</h5>");
    serviceTitle->setTextAlignment(AlignmentFlag::Center);
    serviceTitle->setStyleClass("main-north-widget");
    infoServiceWidget->addWidget(move(serviceTitle));

    _firstRow = infoServiceWidget->addNew<WContainerWidget>();
    _firstRow->setStyleClass("row");

    _signalImg = _firstRow->addNew<WImage>("resources/icons/signal_null.png");
    _signalImg->setStyleClass("col-xs-1 col-md-1 col-lg-1");
    
    _betweenIMG = _firstRow->addNew<WContainerWidget>();
    _betweenIMG->setStyleClass("col-xs-10 col-md-10 col-lg-10");

    auto speaker_img = make_unique<WImage>("resources/icons/speaker.png");
    speaker_img->setStyleClass("col-xs-1 col-md-1 col-lg-1");
    _firstRow->addWidget(move(speaker_img));

    //auto serviceInfoRowInMainWidget = make_unique<WContainerWidget>();
    WContainerWidget* serviceInfoRowInMainWidget = infoServiceWidget->addNew<WContainerWidget>();
    serviceInfoRowInMainWidget->setStyleClass("row");

    //auto serviceInfoContainer = make_unique<WContainerWidget>();
    WContainerWidget *serviceInfoContainer = serviceInfoRowInMainWidget->addNew<WContainerWidget>();
    serviceInfoContainer->setStyleClass("col-xs-12 col-md-12 col-lg-12 text-center");
    
    _ServiceInfo = serviceInfoContainer->addNew<WText>("Kein Service ausgewählt");
    _ServiceInfo->setStyleClass("h1");

    _AntennaImg = serviceInfoContainer->addNew<WImage>("resources/icons/antenna_no_signal.png");

    WContainerWidget *serviceInfoSmallContainer = serviceInfoRowInMainWidget->addNew<WContainerWidget>();
    serviceInfoSmallContainer->setStyleClass("col-xs-12 col-md-12 col-lg-12 text-center");
    
    _ServiceInfoSmall = serviceInfoSmallContainer->addNew<WText>("Kein Service ausgewählt");
    _ServiceInfoSmall->setStyleClass("h4");
    
    if (!app->environment().agentIsMobileWebKit()) 
    {
        // If this would be used from a real Computer/Notebook (Desktop), you could choose 
        // between Audio Streaming through a Browser or direct output over a soundcard 
        // (e.g. HiFi Berry)
        auto mediaPlayerRow = infoServiceWidget->addNew<WContainerWidget>();
        mediaPlayerRow->setStyleClass("row playerframe");

        auto mediaLeftFromPlayerContainer = make_unique<WContainerWidget>();
        mediaLeftFromPlayerContainer->setStyleClass("input-widget col-xs-2 col-md-2 col-lg-2 text-center");

        WSelectionBox *sb = mediaLeftFromPlayerContainer->addNew<WSelectionBox>();
        sb->addItem("Browserstreaming");
        sb->addItem("Intern");
        sb->setCurrentIndex(0);
        
        mediaPlayerRow->addWidget(move(mediaLeftFromPlayerContainer));

        //auto mediaPlayerContainer = mediaPlayerRow->addNew<WContainerWidget>();
        _mediaPlayerContainer = mediaPlayerRow->addNew<WStackedWidget>();
        _mediaPlayerContainer->setStyleClass("col-xs-10 col-md-10 col-lg-10 text-center");
        
        mAudio = _mediaPlayerContainer->addNew<WMediaPlayer>(MediaType::Audio);
        WLink link("/dab/jplayer");
        mAudio->addSource(MediaEncoding::WAV, link);
        mAudio->playbackStarted().connect([=] {
            cout << "Waverider::GUI: Lets go..." << endl;
        });

        _infoTextDirect = _mediaPlayerContainer->addNew<WText>("Soundausgabe direkt über die Soundkarte...");

        sb->activated().connect([=] {
            _mediaPlayerContainer->setCurrentIndex(sb->currentIndex());
        });

        //mediaPlayerRow->addWidget(move(mediaPlayerContainer));

        /* auto mediaRightFromPlayerContainer = make_unique<WContainerWidget>();
        mediaRightFromPlayerContainer->setStyleClass("col-xs-4 col-md-4 col-lg-4 text-center");
        mediaPlayerRow->addWidget(move(mediaRightFromPlayerContainer));*/
        
        //infoServiceWidget->addWidget(move(mediaPlayerRow));
        
        _ChannelContainer->setHeight(800);     
    }
    else
    {
        cout << "IPhone Anpassungen" << endl;
        _ChannelContainer->setHeight(800);
    }

    auto codingLabelRow = infoServiceWidget->addNew<WContainerWidget>();
    codingLabelRow->setStyleClass("row");

    _codingLabel = codingLabelRow->addNew<WText>("DAB+");
    _codingLabel->setStyleClass("col-xs-11 col-xs-offset-11 col-md-11 col-md-offset-11 col-lg-11 col-lg-offset-11");
    
    /** Radio Channels **/
    WText* channels = _ChannelContainer->addNew<WText>("<h3>Channels</h3>");
    channels->setTextAlignment(AlignmentFlag::Center);
    mChannelMenu = _ChannelContainer->addNew<WMenu>();
    
    auto _channelScanItem = make_unique<CStationItem>(0xFF, "Dummy Channel (scan)", "0xFF", nullptr);
    _channelScanItem->setStyleClass("h4");
    // connect vor move!
    _channelScanItem->clicked().connect(this, &WaveriderGUI::scan_dab);
    mChannelMenu->addItem(move(_channelScanItem));    
    
    row_middle->addWidget(move(mMainWidget));
    this->addWidget(move(row_middle));

    /*****************************************************************************************************
     *   MotInfo Widget
     *****************************************************************************************************/
    auto motServiceWidget = mMain_row->addNew<WContainerWidget>();
    motServiceWidget->setStyleClass("mot-service-widget col-xs-12 col-md-12 col-lg-12");
    
    unique_ptr<WText> motTitle = make_unique<WText>("<h5>MOT Slide Show</h5>");
    motTitle->setTextAlignment(AlignmentFlag::Center);
    motTitle->setStyleClass("main-north-widget");
    motServiceWidget->addWidget(move(motTitle));

    _motrow = motServiceWidget->addNew<WContainerWidget>();
    _motrow->setStyleClass("mot-image col-xs-12 col-md-12 col-lg-12 text-center");

    _motImg = _motrow->addNew<WImage>("resources/icons/test-pattern-640.png");
    //_motImg = _motrow->addNew<WImage>("/dab/mot");

    cout << "WaveriderGUI: Channel Container Height: " << _ChannelContainer->height().toPixels() << endl;

    /*****************************************************************************************************
     *   Settings
     *****************************************************************************************************/
    _settings = _mainStack->addNew<CSettingsFormView>(*this);

    /*****************************************************************************************************
     *   Records - Download
     *****************************************************************************************************/
    _records = _mainStack->addNew<CRecords>(*this);

    /*****************************************************************************************************
     *   South Widgets
     *****************************************************************************************************/
    auto last_row = make_unique<WContainerWidget>();
    last_row->setStyleClass("row south-widget");

    auto scan = last_row->addNew<WPushButton>("Scan");
    scan->setStyleClass("controlbutton col-xs-6 col-md-3 col-lg-3");
    scan->clicked().connect(this, &WaveriderGUI::scan_dab);
    
    recordButton = last_row->addNew<WPushButton>("Record");
    recordButton->setStyleClass("controlbutton col-xs-6 col-md-3 col-lg-3");
    recordButton->clicked().connect(this, &WaveriderGUI::record);
    if (_radioServer.isRecording())
    {
        recordButton->setText("");
        recordButton->setIcon("resources/icons/records.gif");
    }
    else
    {
        recordButton->setText("Record");
        recordButton->setIcon("");
    }

    auto stopAudio = last_row->addNew<WPushButton>("Stop");
    stopAudio->setStyleClass("controlbutton col-xs-6 col-md-3 col-lg-3");
    stopAudio->clicked().connect(this, &WaveriderGUI::stopAudio);

    auto quitApp = last_row->addNew<WPushButton>("Quit");
    quitApp->setStyleClass("controlbutton col-xs-6 col-md-3 col-lg-3");
    quitApp->clicked().connect(this, &WaveriderGUI::quit);

    this->addWidget(move(last_row));

    cout << "User uses: " << app->environment().userAgent() << endl;

    // Connect to Waverider-Server, which helds the reference to radio-controller
    connect();

    _stationTimer = app->root()->addChild(make_unique<WTimer>());
    _stationTimer->setInterval(chrono::seconds(1));
    _stationTimer->timeout().connect(this, &WaveriderGUI::initialiseRadioChannels);
    _stationTimer->start();

    // If RadioServer still record, we will continue with radio listening
    playAudio();
}

WaveriderGUI::~WaveriderGUI() 
{
    cout << "Aufräumen" << endl;
}

void WaveriderGUI::connect()
{
    // Server-Push or WebSockets depends on config in wtconfig.xml in /etc
    if (_radioServer.connect(this, 
        bind(&WaveriderGUI::radioEvent, this, placeholders::_1),
        bind(&WaveriderGUI::settingsEvent, this, placeholders::_1)))
    {
        WApplication::instance()->enableUpdates(true);
    }
}

void WaveriderGUI::settingsEvent(const SettingsEvent& e)
{
    WApplication *app = WApplication::instance();

    if (e.getAction() == SettingsAction::loadedSettings)
    {
        _settings->setFormData(e.getDevice(), e.getIPAddress(), e.getPort(), e.getRecordPath());
        _recordPath = e.getRecordPath();

        _records->initDownloadPage();
    }
    else if (e.getAction() == SettingsAction::newSettings)
    {
        _mainStack->setCurrentIndex(0);
        _recordPath = e.getRecordPath();

        _records->initDownloadPage();
    }    

    app->triggerUpdate();
}

void WaveriderGUI::radioEvent(const RadioEvent& event)
{
    WApplication *app = WApplication::instance();

    if (event.getAction() == EventAction::channelFound)
    {
        // add Service
        auto item = make_unique<CStationItem>(event.getServiceID(), event.getLabel(), event.getCurrentChannel(), nullptr);
        item->setStyleClass("h4");
        item->channelClicked().connect(this, &WaveriderGUI::setChannel);
        mChannelMenu->addItem(move(item)); 

        if (_removeChannelScanItem)
        {
            removeChannelScanItem();
        }
    }
    else if (event.getAction() == EventAction::channelChange)
    {
        _ServiceInfo->setText(event.getLabel() + " (Kanal: " + event.getCurrentChannel() + ")");
        _AntennaImg->setImageLink("/resources/icons/antenna.gif");
        _ServiceInfoSmall->setText(event.getLabel());
        mAudio->play();
    }
    else if (event.getAction() == EventAction::motChange)
    {
        //_motrow->removeWidget(_motImg);
        //_motImg = _motrow->addNew<WImage>(event.getLabel());
        _motImg->setImageLink(event.getLabel());
    }
    else if (event.getAction() == EventAction::signalStrength)
    {
        //_firstRow->removeWidget(_signalImg);
        
        //_signalImg = _firstRow->addNew<WImage>("resources/icons/" + event.getSignalStrengthImage());
        //_signalImg = _firstRow->insertNew<WImage>(0, "resources/icons/" + event.getSignalStrengthImage());
        //_signalImg->setStyleClass("col-xs-1 col-md-1 col-lg-1");
        _signalImg->setImageLink("resources/icons/" + event.getSignalStrengthImage());
    }
    else if (event.getAction() == EventAction::channelScan)
    {
        _ServiceInfo->setText("Scanning Channel " + event.getChannelScan() + " ... ");
        _ServiceInfoSmall->setText("Found Channels: " + event.getStationCount());
        _AntennaImg->setImageLink("/resources/icons/antenna.gif");
    }

    app->triggerUpdate();
}

void WaveriderGUI::scan_dab() 
{
    // Delete Scanitem (and others, if exists)
    vector<WMenuItem*> items = mChannelMenu->items();
    for_each(begin(items), end(items), [this](WMenuItem* item) 
    { 
        mChannelMenu->removeItem(item);
    });

    _radioServer.scan_dab();
}

void WaveriderGUI::record()
{
    _radioServer.recordStream();

    if (_radioServer.isRecording()) 
    {
        recordButton->setIcon("resources/icons/records.gif");
        recordButton->setText("");
    }
    else
    {
        recordButton->setIcon("");
        recordButton->setText("Record");
        _records->updateDownloadPage();
    }
}

void WaveriderGUI::removeChannelScanItem()
{
    vector<WMenuItem*> items = mChannelMenu->items();
    for_each(begin(items), end(items), [this](WMenuItem* item) 
    {
        CStationItem* menuitem = (CStationItem*) item; 
        if (menuitem->getChannelID() == "0xFF")
        {
            mChannelMenu->removeItem(item);
            _removeChannelScanItem = false;
        }
    });
}

void WaveriderGUI::initialiseRadioChannels()
{
    // Delete Scanitem (and others, if exists)
    vector<WMenuItem*> items = mChannelMenu->items();
    for_each(begin(items), end(items), [this](WMenuItem* item) 
    {
        CStationItem* menuitem = (CStationItem*) item; 
        if (menuitem->getChannelID() != "0xFF")
        {
            mChannelMenu->removeItem(item);
        }
    });

    cout << "Get saved Channels from DB" << endl; 
    _stationTimer->stop();
    _radioServer.getDABChannels();
    _radioServer.getSettings();
}

void WaveriderGUI::handlePathChange()
{
    WApplication *app = WApplication::instance();
    cout << "Internal Path: " << app->internalPath() << endl;
}

void WaveriderGUI::setChannel(WMenuItem* item)
{
    if (mAudio->playing())
    {
        mAudio->pause();
        _mediaPlayerContainer->removeWidget(mAudio);
        _mediaPlayerContainer->removeWidget(_infoTextDirect);
        
        mAudio = _mediaPlayerContainer->addNew<WMediaPlayer>(MediaType::Audio);
        WLink link("/dab/jplayer");
        mAudio->addSource(MediaEncoding::WAV, link);
        mAudio->playbackStarted().connect([=] {
            cout << "Waverider::GUI: Lets go..." << endl;
        });

        _infoTextDirect = _mediaPlayerContainer->addNew<WText>("Soundausgabe direkt über die Soundkarte...");
    }

    CStationItem* sitem = (CStationItem*) item;
    cout << "Clicked:"
         << " serviceID: " << sitem->getServiceIDASHexString() 
         << " serviceName: " << sitem->getServiceName() 
         << " channelID: " << sitem->getChannelID() << endl; 

    _radioServer.setChannel(sitem->getServiceID(), sitem->getServiceName(), sitem->getChannelID());
}

void WaveriderGUI::playAudio()
{
    if (_radioServer.isRecording() || _radioServer.isPlaying())
    {
        _radioServer.reactivateRecordingChannel();

        _mediaPlayerContainer->removeWidget(mAudio);
        _mediaPlayerContainer->removeWidget(_infoTextDirect);
            
        mAudio = _mediaPlayerContainer->addNew<WMediaPlayer>(MediaType::Audio);
        WLink link("/dab/jplayer");
        mAudio->addSource(MediaEncoding::WAV, link);
        mAudio->playbackStarted().connect([=] {
            cout << "Waverider::GUI: Lets go..." << endl;
        });
        mAudio->play();

        _infoTextDirect = _mediaPlayerContainer->addNew<WText>("Soundausgabe direkt über die Soundkarte...");
    }
}

void WaveriderGUI::stopAudio()
{
    mAudio->stop();

    _radioServer.stop();
}

void WaveriderGUI::quit()
{
    this->stopAudio();

    cout << "Quit the App" << endl;
    WApplication *app = WApplication::instance();
    app->quit();
}