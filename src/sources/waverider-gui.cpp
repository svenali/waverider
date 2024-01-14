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
    //cout << "APP ID: " << app->id() << endl;

    // Standard dab+ - Modus
    _dabMode = true;
    _bouquetMode = false;

    _removeChannelScanItem = true;
    _removeWebChannelScanItem = true;
    _lastSelectedRadioStation = nullptr;

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

    auto bouquetEditorButton = make_unique<WPushButton>();
    bouquetEditorButton->setIcon("resources/icons/flower-bouquet-32x32.png");
    bouquetEditorButton->setStyleClass("north-widget col-xs-1 col-md-1 col-lg-1");

    // Modi umschalten
    _WebAndDABSwitchButton = header_row->addNew<WPushButton>();
    _WebAndDABSwitchButton->setIcon("resources/icons/internet-radio-fm-32x32.png");
    _WebAndDABSwitchButton->setStyleClass("north-widget col-xs-6 col-md-1 col-lg-1");
    if (app->environment().agentIsMobileWebKit()) 
    {
        _WebAndDABSwitchButton->setIcon("resources/icons/internet-radio-fm-154x130.png");
        _WebAndDABSwitchButton->setHeight(150);
    }

    // Bouquets
    auto bouquetsButton = header_row->addNew<WPushButton>();
    bouquetsButton->setIcon("resources/icons/favoriten-32x32.png");
    bouquetsButton->setStyleClass("north-widget col-xs-6 col-md-1 col-lg-1");
    if (app->environment().agentIsMobileWebKit()) 
    {
        bouquetsButton->setIcon("resources/icons/favoriten-137x130.png");
        bouquetsButton->setHeight(150);
    }
    
    if (!app->environment().agentIsMobileWebKit()) 
    {
        unique_ptr<WText> title = make_unique<WText>("<h2>Waverider</h2>");
        title->setTextAlignment(AlignmentFlag::Center);
        title->setStyleClass("north-widget col-xs-7 col-md-7 col-lg-7");
        header_row->addWidget(std::move(title));
    }
    
    auto row_middle = make_unique<WContainerWidget>();
    row_middle->setStyleClass("row");
    
    _channelContainer = row_middle->addNew<WStackedWidget>();
    _channelContainer->setStyleClass("west-widget col-xs-12 col-md-3 col-lg-3");

    _DABChannelContainer = _channelContainer->addNew<WContainerWidget>();
    _InternetChannelContainer = _channelContainer->addNew<WContainerWidget>();
    _BouquetChannelContainer = _channelContainer->addNew<WContainerWidget>();
    WText* web_channel_text = _InternetChannelContainer->addNew<WText>("<h3>web Channels</h3>");
    web_channel_text->setTextAlignment(AlignmentFlag::Center);
    WContainerWidget *web_channels = _InternetChannelContainer->addNew<WContainerWidget>();
    web_channels->setHeight(500);
    mWebChannelMenu = web_channels->addNew<WMenu>();
    auto _channelWebScanItem = make_unique<CStationItem>(0xFF, "Dummy Channel (download)", "0xFF", nullptr);
    _channelWebScanItem->setStyleClass("h4");
    // connect vor move!
    _channelWebScanItem->clicked().connect(this, &WaveriderGUI::scan_internet);
    mWebChannelMenu->addItem(std::move(_channelWebScanItem)); 
    
    mMainWidget = make_unique<WContainerWidget>();
    mMainWidget->setStyleClass("east-widget col-xs-12 col-md-9 col-lg-9");

    /*********************************************************************************************
     *   Settings and Services
     ********************************************************************************************/
    _mainStack = mMainWidget->addNew<WStackedWidget>();
    settingsButton->clicked().connect([=] 
    {
        if (_mainStack->currentIndex() == 0 || _mainStack->currentIndex() == 2 || _mainStack->currentIndex() == 3 )
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
        if (_mainStack->currentIndex() == 0 || _mainStack->currentIndex() == 1 || _mainStack->currentIndex() == 3 )
        {
            _mainStack->setCurrentIndex(2);
        }
        else
        {
            _mainStack->setCurrentIndex(0);
        }
    });
    bouquetEditorButton->clicked().connect([=]
    {
        if (_mainStack->currentIndex() == 0 || _mainStack->currentIndex() == 1 || _mainStack->currentIndex() == 2)
        {
            _mainStack->setCurrentIndex(3);

            getRadioServer().getSavedDABChannels();
        }
        else
        {
            _mainStack->setCurrentIndex(0);
        }    
    });

    _WebAndDABSwitchButton->clicked().connect([=]
    {
        if (_dabMode)
        {
            _dabMode = false;
            _bouquetMode = false;
            if (app->environment().agentIsMobileWebKit()) 
                _WebAndDABSwitchButton->setIcon("resources/icons/DABplus_Logo_Farbe_sRGB-221x130.png");
            else
                _WebAndDABSwitchButton->setIcon("resources/icons/DABplus_Logo_Farbe_sRGB-32x32.png");
            _channelContainer->setCurrentIndex(1);
        }
        else
        {
            _dabMode = true;
            _bouquetMode = false;
            if (app->environment().agentIsMobileWebKit())
                _WebAndDABSwitchButton->setIcon("resources/icons/internet-radio-fm-154x130.png");
            else
                _WebAndDABSwitchButton->setIcon("resources/icons/internet-radio-fm-32x32.png");
            _channelContainer->setCurrentIndex(0);
        }
    });

    bouquetsButton->clicked().connect([=]
    {
        _channelContainer->setCurrentIndex(2);
        _bouquetMode = true;
    });

    if (!app->environment().agentIsMobileWebKit()) 
    {
        header_row->addWidget(std::move(bouquetEditorButton));
        header_row->addWidget(std::move(fileDownloadButton));
        header_row->addWidget(std::move(settingsButton));
    }
    
    this->addWidget(std::move(header_row));

    /*********************************************************************************************
     *   Service Info
     ********************************************************************************************/
    //auto mMain_row = mMainWidget->addNew<WContainerWidget>();
    auto mMain_row = _mainStack->addNew<WContainerWidget>();
    mMain_row->setStyleClass("row");

    auto infoServiceWidget = mMain_row->addNew<WContainerWidget>();
    infoServiceWidget->setStyleClass("info-service-widget col-xs-12 col-md-12 col-lg-12");

    unique_ptr<WText> serviceTitle = make_unique<WText>("<h5>Service Overview</h5>");
    serviceTitle->setTextAlignment(AlignmentFlag::Center);
    serviceTitle->setStyleClass("main-north-widget");
    infoServiceWidget->addWidget(std::move(serviceTitle));

    _firstRow = infoServiceWidget->addNew<WContainerWidget>();
    _firstRow->setStyleClass("row");

    _signalImg = _firstRow->addNew<WImage>("resources/icons/signal_null.png");
    _signalImg->setStyleClass("col-xs-1 col-md-1 col-lg-1");
    
    _betweenIMG = _firstRow->addNew<WContainerWidget>();
    _betweenIMG->setStyleClass("col-xs-10 col-md-10 col-lg-10");

    auto speaker_img = make_unique<WImage>("resources/icons/speaker.png");
    speaker_img->setStyleClass("col-xs-1 col-md-1 col-lg-1");
    _firstRow->addWidget(std::move(speaker_img));

    //auto serviceInfoRowInMainWidget = make_unique<WContainerWidget>();
    WContainerWidget* serviceInfoRowInMainWidget = infoServiceWidget->addNew<WContainerWidget>();
    serviceInfoRowInMainWidget->setStyleClass("row");

    //auto serviceInfoContainer = make_unique<WContainerWidget>();
    _serviceInfoContainer = serviceInfoRowInMainWidget->addNew<WContainerWidget>();
    _serviceInfoContainer->setStyleClass("col-xs-12 col-md-12 col-lg-12 text-center");
    
    _ServiceInfo = _serviceInfoContainer->addNew<WText>("Kein Service ausgewählt");
    _ServiceInfo->setStyleClass("h1");

    _AntennaImg = _serviceInfoContainer->addNew<WImage>("resources/icons/antenna_no_signal.png");

    _serviceInfoSmallContainer = serviceInfoRowInMainWidget->addNew<WContainerWidget>();
    _serviceInfoSmallContainer->setStyleClass("col-xs-12 col-md-12 col-lg-12 text-center");
    
    _ServiceInfoSmall = _serviceInfoSmallContainer->addNew<WText>("Kein Service ausgewählt");
    _ServiceInfoSmall->setStyleClass("h4");
    
    if (!app->environment().agentIsMobileWebKit()) 
    {
        // If this would be used from a real Computer/Notebook (Desktop), you could choose 
        // between Audio Streaming through a Browser or direct output over a soundcard 
        // (e.g. HiFi Berry)
        auto mediaPlayerRow = infoServiceWidget->addNew<WContainerWidget>();
        mediaPlayerRow->setStyleClass("row playerframe justify-content-center");

        auto mediaLeftFromPlayerContainer = make_unique<WContainerWidget>();
        mediaLeftFromPlayerContainer->setStyleClass("input-widget col-xs-4 col-md-4 col-lg-4");

        _audioDevice = mediaLeftFromPlayerContainer->addNew<WSelectionBox>();
        _audioDevice->addItem("Browser");
        /* sb->addItem("Intern");
        sb->setCurrentIndex(0);*/
        
        mediaPlayerRow->addWidget(std::move(mediaLeftFromPlayerContainer));

        //auto mediaPlayerContainer = mediaPlayerRow->addNew<WContainerWidget>();
        _mediaPlayerContainer = mediaPlayerRow->addNew<WStackedWidget>();
        _mediaPlayerContainer->setStyleClass("col-xs-8 col-md-8 col-lg-8");
        
        mAudio = _mediaPlayerContainer->addNew<WMediaPlayer>(MediaType::Audio);
        /* mAudio->playbackStarted().connect([=] {
            cout << "Waverider::GUI: Lets go..." << endl;
        }); */

        _infoTextDirect = _mediaPlayerContainer->addNew<WText>("Soundausgabe direkt über die Soundkarte...");

        _audioDevice->activated().connect([=] {
            _mediaPlayerContainer->setCurrentIndex(_audioDevice->currentIndex());
        });

        //mediaPlayerRow->addWidget(move(mediaPlayerContainer));

        /* auto mediaRightFromPlayerContainer = make_unique<WContainerWidget>();
        mediaRightFromPlayerContainer->setStyleClass("col-xs-4 col-md-4 col-lg-4 text-center");
        mediaPlayerRow->addWidget(move(mediaRightFromPlayerContainer));*/
        
        //infoServiceWidget->addWidget(move(mediaPlayerRow));
        
        _channelContainer->setHeight(800);     
    }
    else
    {
        // IPhone Anpassungen"
        _channelContainer->setHeight(500);
    }

    auto codingLabelRow = infoServiceWidget->addNew<WContainerWidget>();
    codingLabelRow->setStyleClass("row");

    _codingLabel = codingLabelRow->addNew<WText>("DAB+");
    _codingLabel->setStyleClass("col-xs-11 col-xs-offset-11 col-md-11 col-md-offset-11 col-lg-11 col-lg-offset-11");
    
    /** Radio Channels **/
    WText* channels = _DABChannelContainer->addNew<WText>("<h3>dab+ Channels</h3>");
    channels->setTextAlignment(AlignmentFlag::Center);
    mChannelMenu = _DABChannelContainer->addNew<WMenu>();
    
    auto _channelScanItem = make_unique<CStationItem>(0xFF, "Dummy Channel (scan)", "0xFF", nullptr);
    _channelScanItem->setStyleClass("h4");
    // connect vor move!
    _channelScanItem->clicked().connect(this, &WaveriderGUI::scan_dab);
    mChannelMenu->addItem(std::move(_channelScanItem));    
    
    row_middle->addWidget(std::move(mMainWidget));
    this->addWidget(std::move(row_middle));

    /** Bouquet Channels */
    _favoriteText = _BouquetChannelContainer->addNew<WText>("<h3>Favoriten</h3>");
    _favoriteText->setTextAlignment(AlignmentFlag::Center);
    mBouquetChannelMenu = _BouquetChannelContainer->addNew<WMenu>();

    /********************************************************************************************
    *   MotInfo Widget
    ********************************************************************************************/
    auto motServiceWidget = mMain_row->addNew<WContainerWidget>();
    motServiceWidget->setStyleClass("mot-service-widget col-xs-12 col-md-12 col-lg-12");
    
    unique_ptr<WText> motTitle = make_unique<WText>("<h5>MOT Slide Show</h5>");
    motTitle->setTextAlignment(AlignmentFlag::Center);
    motTitle->setStyleClass("main-north-widget");
    motServiceWidget->addWidget(std::move(motTitle));

    _motrow = motServiceWidget->addNew<WContainerWidget>();
    _motrow->setStyleClass("mot-image col-xs-12 col-md-12 col-lg-12 text-center");

    _motImg = _motrow->addNew<WImage>("resources/icons/test-pattern-640.png");
    _motImg->setStyleClass("motimg-himself");
    //_motImg = _motrow->addNew<WImage>("/dab/mot");

    /*********************************************************************************************
     *   Settings
     *******************************************************************************************/
    _settings = _mainStack->addNew<CSettingsFormView>(*this);

    /*********************************************************************************************
     *   Records - Download
     ********************************************************************************************/
    _records = _mainStack->addNew<CRecords>(*this);

    /*********************************************************************************************
     *   Bouqueteditor
     *******************************************************************************************/
    _bouqueteditor = _mainStack->addNew<CBouquetEditorView>(*this);

    /*********************************************************************************************
     *   South Widgets
     ********************************************************************************************/
    auto last_row = make_unique<WContainerWidget>();
    last_row->setStyleClass("row south-widget");

    if (!app->environment().agentIsMobileWebKit()) 
    {
        auto scan = last_row->addNew<WPushButton>("Scan");
        scan->setStyleClass("controlbutton col-xs-6 col-md-3 col-lg-3");
        scan->clicked().connect(this, &WaveriderGUI::scan_dab);
    }
    
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

    if (!app->environment().agentIsMobileWebKit()) 
    {
        auto quitApp = last_row->addNew<WPushButton>("Quit");
        quitApp->setStyleClass("controlbutton col-xs-6 col-md-3 col-lg-3");
        quitApp->clicked().connect(this, &WaveriderGUI::quit);
    }

    this->addWidget(std::move(last_row));

    _radioServer.log("notice", "User uses: " + app->environment().userAgent());

    // Connect to Waverider-Server, which helds the reference to radio-controller
    connect();

    _stationTimer = app->root()->addChild(make_unique<WTimer>());
    _stationTimer->setInterval(chrono::seconds(1));
    _stationTimer->timeout().connect(this, &WaveriderGUI::initialiseRadioChannels);
    _stationTimer->start();

    // If RadioServer still record, we will continue with radio listening
    playAudio();

    // Testing Internetradio
    //_radioServer.setInternetChannel("");
}

WaveriderGUI::~WaveriderGUI() 
{
    _radioServer.log("notice", " Client finished with cleanup ...");
}

void WaveriderGUI::connect()
{
    // Server-Push or WebSockets depends on config in wtconfig.xml in /etc
    if (_radioServer.connect(this, 
        bind(&WaveriderGUI::radioEvent, this, placeholders::_1),
        bind(&WaveriderGUI::settingsEvent, this, placeholders::_1),
        bind(&WaveriderGUI::bouquetEvent, this, placeholders::_1),
        bind(&WaveriderGUI::recordEvent, this, placeholders::_1),
        bind(&WaveriderGUI::errorEvent, this, placeholders::_1)))
    {
        WApplication::instance()->enableUpdates(true);
    }
}

void WaveriderGUI::recordEvent(const RecordFileEvent& event)
{
    _records->initContentPage(event.getResult());
}

void WaveriderGUI::errorEvent(const ErrorEvent& event)
{
    WApplication *app = WApplication::instance();

    auto wmb = make_unique<WMessageBox>("Error", event.getMessage(), Icon::Critical, StandardButton::Ok);
    wmb->show();
    WPushButton *ok = wmb->button(StandardButton::Ok);
    ok->clicked().connect(wmb.get(), &Wt::WDialog::accept);
    //wmb.get()->finished().connect(this, &MyClass::dialogDone);
    this->addChild(std::move(wmb));

    recordButton->setIcon("");
    recordButton->setText("Record");

    app->triggerUpdate();
}

void WaveriderGUI::bouquetEvent(const BouquetEditorEvent& e)
{
    WApplication *app = WApplication::instance();
    if (app->environment().agentIsMobileWebKit()) 
        return;

    if (e.getAction() == BouquetEditorEventAction::savedChannels)
    {
        const map<string, string> channels = e.getChannelList();

        _bouqueteditor->getFormModel()->updateWebChannels(channels);
    }
    else if (e.getAction() == BouquetEditorEventAction::savedCountries)
    {
        _bouqueteditor->getFormModel()->updateFilter2(e.getResult());
    }
    else if (e.getAction() == BouquetEditorEventAction::bouquetsLoaded)
    {
        _bouqueteditor->getFormModel()->loadBouquetsFromDB(e.getBouquets());
    }
    else if (e.getAction() == BouquetEditorEventAction::savedExactCountryCodes)
    {
        _bouqueteditor->getFormModel()->updateFilter2(e.getResult());    
    }
    else if (e.getAction() == BouquetEditorEventAction::savedCodecs)
    {
        _bouqueteditor->getFormModel()->updateFilter2(e.getResult());    
    }
    else if (e.getAction() == BouquetEditorEventAction::savedLanguages)
    {
        _bouqueteditor->getFormModel()->updateFilter2(e.getResult());    
    }
    else if (e.getAction() == BouquetEditorEventAction::savedTags)
    {
        _bouqueteditor->getFormModel()->updateFilter2(e.getResult());    
    }
    else if (e.getAction() == BouquetEditorEventAction::savedDABChannels)
    {
        const map<string, string> channels = e.getChannelList();

        _bouqueteditor->getFormModel()->updateDabChannels(channels);    
    }
    else if (e.getAction() == BouquetEditorEventAction::newBouquets)
    {
        _channelContainer->setCurrentIndex(2);
        _bouquetMode = true;

        _mainStack->setCurrentIndex(0);

        vector<WMenuItem*> items = mBouquetChannelMenu->items();
        for_each(begin(items), end(items), [this](WMenuItem* item) 
        {
            CStationItem* menuitem = (CStationItem*) item; 
            mBouquetChannelMenu->removeItem(item);
        });

        _radioServer.getBouquets();
    }
}

void WaveriderGUI::settingsEvent(const SettingsEvent& e)
{
    WApplication *app = WApplication::instance();

    if (app->environment().agentIsMobileWebKit()) 
    {
        // Not on iPhones
        app->triggerUpdate();
        return;
    }

    if (e.getAction() == SettingsAction::loadedSettings)
    {
        _settings->setFormData(e.getDevice(), e.getIPAddress(), e.getPort(), e.getRecordPath(), e.getRadioBrowserURL(), e.getDoNotReEncode(), e.getStreaming(), e.getStreamingAddress(), e.getStreamingFormat(), e.getRecordFormat(), e.getRetrieveMetadata(), e.getSaveMetadata());
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

        if (!_bouquetMode)
            mChannelMenu->addItem(std::move(item)); 
        else
            mBouquetChannelMenu->addItem(std::move(item));

        if (_removeChannelScanItem)
        {
            removeChannelScanItem();
        }
    }
    else if (event.getAction() == EventAction::webChannelFound)
    {
        // add WebService
        auto item = make_unique<CStationItem>(event.getLabel(), event.getWebURL(), nullptr);
        item->setStyleClass("h4");
        item->channelClicked().connect(this, &WaveriderGUI::setWebChannel);

        if (!_bouquetMode)
            mWebChannelMenu->addItem(std::move(item));
        else
            mBouquetChannelMenu->addItem(std::move(item));

        if (_removeWebChannelScanItem)
        {
            removeWebChannelScanItem();
        }
    }
    else if (event.getAction() == EventAction::createNextLink)
    {
        // add WebService
        auto itemNext = make_unique<CStationItem>("Next", "", nullptr);
        itemNext->setStyleClass("h4");
        itemNext->channelClicked().connect(this, &WaveriderGUI::setWebChannel);
        mWebChannelMenu->addItem(std::move(itemNext));
    }
    else if (event.getAction() == EventAction::removeNextLink)
    {
        // Delete Scanitem (and others, if exists)
        vector<WMenuItem*> items = mWebChannelMenu->items();
        for_each(begin(items), end(items), [this](WMenuItem* item) 
        { 
            CStationItem *sitem = (CStationItem*) item;
            if (sitem->getServiceName() == "Next")
                mWebChannelMenu->removeItem(item);
        });    
    }
    else if (event.getAction() == EventAction::createPrevLink)
    {
        auto itemPrev = make_unique<CStationItem>("Prev", "", nullptr);
        itemPrev->setStyleClass("h4");
        itemPrev->channelClicked().connect(this, &WaveriderGUI::setWebChannel);
        mWebChannelMenu->addItem(std::move(itemPrev));
    }
    else if (event.getAction() == EventAction::removePrevLink)
    {
        // Delete Scanitem (and others, if exists)
        vector<WMenuItem*> items = mWebChannelMenu->items();
        for_each(begin(items), end(items), [this](WMenuItem* item) 
        { 
            CStationItem *sitem = (CStationItem*) item;
            if (sitem->getServiceName() == "Prev")
                mWebChannelMenu->removeItem(item);
        });
    }
    else if (event.getAction() == EventAction::bouquetFound)
    {
        // add Bouquet
        auto item = make_unique<CStationItem>(event.getSignalStrengthImage(), nullptr);
        cout << "Bouequet: " << event.getSignalStrengthImage() << endl;
        item->setStyleClass("h4");
        item->channelClicked().connect(this, &WaveriderGUI::loadBouquetChannels);
        mBouquetChannelMenu->addItem(std::move(item));   
    }
    else if (event.getAction() == EventAction::backToBouquets)
    {
        auto itemPrev = make_unique<CStationItem>("Prev", "", nullptr);
        itemPrev->setStyleClass("h4");
        itemPrev->channelClicked().connect(this, &WaveriderGUI::loadBouquets);
        mBouquetChannelMenu->addItem(std::move(itemPrev));   
    }
    else if (event.getAction() == EventAction::channelChange)
    {
        _ServiceInfo->setText(event.getLabel() + " (Kanal: " + event.getCurrentChannel() + ")");
        _AntennaImg->setImageLink("/resources/icons/antenna.gif");
        _ServiceInfoSmall->setText(event.getLabel());

        _motImg->setImageLink("resources/icons/test-pattern-640-geduld.png");

        /* _mediaPlayerContainer->removeWidget(mAudio);
        _mediaPlayerContainer->removeWidget(_infoTextDirect);
        
        mAudio = _mediaPlayerContainer->addNew<WMediaPlayer>(MediaType::Audio);
        WLink link("/dab/jplayer");
        mAudio->addSource(MediaEncoding::WAV, link);
        mAudio->playbackStarted().connect([=] {
            cout << "Waverider::GUI: Lets go..." << endl;
        });

        mAudio->play(); -> not yet! It often happens, that no audio is coming!!! Wait till Audio */
    }
    else if (event.getAction() == EventAction::receiveAudio)
    {
        if (!app->environment().agentIsMobileWebKit()) 
        {
            _radioServer.log("notice", "[CWaveriderGUI::radioEvent] mAudio->play()");

            _mediaPlayerContainer->removeWidget(mAudio);
            _mediaPlayerContainer->removeWidget(_infoTextDirect);

            mAudio = _mediaPlayerContainer->addNew<WMediaPlayer>(MediaType::Audio);
            WLink link("/dab/jplayer");
            mAudio->addSource(MediaEncoding::WAV, link);
            /* mAudio->playbackStarted().connect([=] {
                cout << "Waverider::GUI: Lets go..." << endl;
            });*/

            mAudio->play(); 
        }    

        _motImg->setImageLink("resources/icons/test-pattern-640.png");
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
    else if (event.getAction() == EventAction::newTitle)
    {
        _ServiceInfoSmall->setText(event.getNewTitle());
    }
    else if (event.getAction() == EventAction::channelScan)
    {
        _ServiceInfo->setText("Scanning Channel " + event.getChannelScan() + " ... ");
        _ServiceInfoSmall->setText("Found Channels: " + event.getStationCount());
        _AntennaImg->setImageLink("/resources/icons/antenna.gif");
    }
    else if (event.getAction() == EventAction::internetCountryCount)
    {
        auto progressBar = (WProgressBar*) _serviceInfoSmallContainer->widget(_progressBarIndex);
        progressBar->setRange(0.0, 100.0);
        _internetCountedCountries = event.getCountryCount();
    }
    else if (event.getAction() == EventAction::internetCountryScan)
    {
        _ServiceInfo->setText("Scanning Channel " + event.getChannelScan() + " (" + event.getLabel() + ")... "); 

        auto progressBar = (WProgressBar*) _serviceInfoSmallContainer->widget(_progressBarIndex);
        progressBar->setValue((event.getCountryCount() / (double)_internetCountedCountries)*100);
    }
    else if (event.getAction() == EventAction::internetScanFinished)
    {
        _ServiceInfo->setText("Kein Service ausgewählt");
        _ServiceInfoSmall->setText("Kein Service ausgewählt");

        auto progressBar = (WProgressBar*) _serviceInfoSmallContainer->widget(_progressBarIndex);
        _serviceInfoSmallContainer->removeWidget(progressBar);

        _radioServer.getWebChannels(InternetChannelScroll::nothing);
    }
    else if (event.getAction() == EventAction::soundcardFound)
    {
        _audioDevice->addItem(event.getSoundDevice());
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

void WaveriderGUI::scan_internet()
{
    // Remove Text and show a progress bar
    _ServiceInfoSmall->setText("");
    auto progressBar = _serviceInfoSmallContainer->addNew<WProgressBar>();
    _progressBarIndex = _serviceInfoSmallContainer->indexOf(progressBar);

    // Delete Scanitem (and others, if exists)
    vector<WMenuItem*> items = mWebChannelMenu->items();
    for_each(begin(items), end(items), [this](WMenuItem* item) 
    { 
        mWebChannelMenu->removeItem(item);
    });

    _radioServer.scan_internet();   
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

void WaveriderGUI::removeWebChannelScanItem()
{
    vector<WMenuItem*> items = mWebChannelMenu->items();
    for_each(begin(items), end(items), [this](WMenuItem* item) 
    {
        CStationItem* menuitem = (CStationItem*) item; 
        if (menuitem->getChannelID() == "0xFF")
        {
            mWebChannelMenu->removeItem(item);
            _removeWebChannelScanItem = false;
        }
    });
}

void WaveriderGUI::initialiseRadioChannels()
{
    WApplication *app = WApplication::instance();

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

    _stationTimer->stop();
    _radioServer.getDABChannels();
    _radioServer.getWebChannels(InternetChannelScroll::nothing);
    _radioServer.getBouquets();
    
    if (!app->environment().agentIsMobileWebKit()) 
    {
        _radioServer.getSettings();
        _radioServer.getBouquetsInDB();   
    }

#if defined(HAVE_ALSA)
    if (!app->environment().agentIsMobileWebKit()) 
    {
        _radioServer.getRadioController()->postFoundedSoundCards();
    }
#endif
}

void WaveriderGUI::handlePathChange()
{
    WApplication *app = WApplication::instance();
    _radioServer.log("notice", "Internal Path: " + app->internalPath());
}

void WaveriderGUI::setChannel(WMenuItem* item)
{
    WApplication *app = WApplication::instance();

    if (!app->environment().agentIsMobileWebKit()) 
    {
        if (mAudio->playing())
        {
            mAudio->pause();

            _infoTextDirect = _mediaPlayerContainer->addNew<WText>("Soundausgabe direkt über die Soundkarte...");
        }
    }

    // Does the Server still plays because of an another frontend?
    if (_radioServer.isPlaying())
    {
        _radioServer.shutup();
    }
    
    CStationItem* sitem = (CStationItem*) item;
    
    _lastSelectedRadioStation = sitem;

    _radioServer.setChannel(sitem->getServiceID(), sitem->getServiceName(), sitem->getChannelID());
}

void WaveriderGUI::setWebChannel(WMenuItem* item)
{
    CStationItem* sitem = (CStationItem*) item;

    if (sitem->getServiceName().compare("Prev") == 0)
    {
        vector<WMenuItem*> items = mWebChannelMenu->items();
        for_each(begin(items), end(items), [this](WMenuItem* item) 
        {
            CStationItem* menuitem = (CStationItem*) item; 
            mWebChannelMenu->removeItem(item);
        });
        _radioServer.getWebChannels(InternetChannelScroll::prev);    
    }
    else if (sitem->getServiceName().compare("Next") == 0)
    {
        vector<WMenuItem*> items = mWebChannelMenu->items();
        for_each(begin(items), end(items), [this](WMenuItem* item) 
        {
            CStationItem* menuitem = (CStationItem*) item; 
            mWebChannelMenu->removeItem(item);
        });
        _radioServer.getWebChannels(InternetChannelScroll::forward);
    }
    else
    {
        WApplication *app = WApplication::instance();
        if (!app->environment().agentIsMobileWebKit()) 
        {
            if (mAudio->playing())
            {
                mAudio->pause();

                _infoTextDirect = _mediaPlayerContainer->addNew<WText>("Soundausgabe direkt über die Soundkarte...");
            }
        }

        _radioServer.shutup();

        _lastSelectedRadioStation = sitem;

        _radioServer.setWebChannel(sitem->getServiceName(), sitem->getWebURL());
    }
}

void WaveriderGUI::loadBouquetChannels(WMenuItem* item)
{
    CStationItem* sitem = (CStationItem*) item;
    string bouquetName = sitem->getServiceName();
    
    vector<WMenuItem*> items = mBouquetChannelMenu->items();
    for_each(begin(items), end(items), [this](WMenuItem* wmi) 
    {
        CStationItem* menuitem = (CStationItem*) wmi; 
        cout << "Removing " << menuitem->getServiceName() << endl;
        mBouquetChannelMenu->removeItem(wmi);
    });

    _favoriteText->setText("<h3>Favoriten: " + bouquetName + "</h3>");
    _radioServer.loadChannelsInBouquet(bouquetName);
}

void WaveriderGUI::loadBouquets()
{
    vector<WMenuItem*> items = mBouquetChannelMenu->items();
    for_each(begin(items), end(items), [this](WMenuItem* item) 
    {
        CStationItem* menuitem = (CStationItem*) item; 
        mBouquetChannelMenu->removeItem(item);
    });

    _favoriteText->setText("<h3>Favoriten</h3>");

    _radioServer.getBouquets();
}

void WaveriderGUI::playAudio()
{
    WApplication *app = WApplication::instance();

    if (_radioServer.isRecording() || _radioServer.isPlaying())
    {
        _radioServer.reactivateRecordingChannel();

        if (!app->environment().agentIsMobileWebKit()) 
        {
            _mediaPlayerContainer->removeWidget(mAudio);
            _mediaPlayerContainer->removeWidget(_infoTextDirect);
                
            mAudio = _mediaPlayerContainer->addNew<WMediaPlayer>(MediaType::Audio);
            WLink link("/dab/jplayer");
            mAudio->addSource(MediaEncoding::WAV, link);
            /* mAudio->playbackStarted().connect([=] {
                cout << "Waverider::GUI: Lets go..." << endl;
            });*/
            mAudio->play();

            _infoTextDirect = _mediaPlayerContainer->addNew<WText>("Soundausgabe direkt über die Soundkarte...");
        }
    }
}

void WaveriderGUI::stopAudio()
{
    WApplication *app = WApplication::instance();
    if (!app->environment().agentIsMobileWebKit()) 
    {
        mAudio->stop();
    }

    /* if (_lastSelectedRadioStation != nullptr)
    {
        if (_lastSelectedRadioStation->getBroadcastType() == "dab+")
            _radioServer.stop();
        else 
            _radioServer.webStop();
    }
    else
    {*/
        if (_radioServer.isPlaying())
        {
            // WebApp was started with an other device
            _radioServer.shutup();
        }
    //}
}

void WaveriderGUI::quit()
{
    this->stopAudio();

    /* if (_lastSelectedRadioStation != nullptr)
    {
        if (_lastSelectedRadioStation->getBroadcastType() == "dab+")
            _radioServer.stop();
        else 
            _radioServer.webStop();
    } */

    if (_radioServer.isPlaying())
    {
        // WebApp was started with an other device
        _radioServer.shutup();
    }

    _radioServer.log("notice", "Client quits the App ...");

    WApplication *app = WApplication::instance();
    app->quit();
}