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

#include "cbouqueteditorview.h"
#include "cbouqueteditorform.h"

CBouquetEditorView::CBouquetEditorView(WaveriderGUI& gui)
:   WTemplateFormView(),
    _rider_gui(gui)
{
    _form = make_shared<CBouquetEditorForm>();

    setTemplateText(tr("bouqueteditor"));
    addFunction("id", &WTemplate::Functions::id);
    addFunction("block", &WTemplate::Functions::id);

    auto bouquets = make_unique<WSelectionBox>();
    bouquets->setModel(_form->getbouquet());
    auto bouquets_ptr = bouquets.get();
    //bindWidget("bouquets", move(bouquets));
    setFormWidget(CBouquetEditorForm::bouquets, std::move(bouquets),
    [=]{
        // updateViewValue()
        string title = asString(_form->value(CBouquetEditorForm::bouquets)).toUTF8();
		int row = _form->bouquetRowModel(title);
		bouquets_ptr->setCurrentIndex(row);
    },
    [=]{
        // updateModelValue()
        string code = _form->bouquet(bouquets_ptr->currentIndex()); 
		_form->setValue(CBouquetEditorForm::bouquets, code);
    }); 

    auto bouquet_line = make_unique<WLineEdit>();
    bouquet_line->setWidth("200px");

    bindWidget("bouquet-name", std::move(bouquet_line));

    auto button = bindWidget("add-bouquet-button", make_unique<WPushButton>("Hinzufügen"));
    button->clicked().connect(this, &CBouquetEditorView::newBouquet);

    auto remove = bindWidget("bouquet-remove-button", make_unique<WPushButton>("Entfernen"));
    remove->clicked().connect(this, &CBouquetEditorView::removeBouquet);

    // Channels to Bouquets
    auto bouquet_selection = make_unique<WComboBox>();
    bouquet_selection->setModel(_form->getbouquetSelection());
    bouquet_selection->activated().connect(this, &CBouquetEditorView::showChannelsInBouquet);
    bouquet_selection->clicked().connect(this, &CBouquetEditorView::channelInBouquetClicked);
    auto bouquet_selection_ptr = bouquet_selection.get();
    //bindWidget("bouquet-selection", move(bouquet_selection));
    setFormWidget(CBouquetEditorForm::bouquet_selection, std::move(bouquet_selection),
    [=]{
        // updateViewValue()
        string title = asString(_form->value(CBouquetEditorForm::bouquet_selection)).toUTF8();
		int row = _form->bouquetSelectionRowModel(title);
		bouquet_selection_ptr->setCurrentIndex(row);
    },
    [=]{
        // updateModelValue()
        string code = _form->bouquetSelection(bouquet_selection_ptr->currentIndex()); 
		_form->setValue(CBouquetEditorForm::bouquet_selection, code);
    });

    auto channels_in_bouquet = make_unique<WSelectionBox>();
    //channels_in_bouquet->setSelectionMode(Wt::SelectionMode::Extended);
    channels_in_bouquet->setModel(_form->getChannelsInBouquet());
    auto channels_in_bouquet_ptr = channels_in_bouquet.get();
    //bindWidget("channels-in-bouquet", move(channels_in_bouquet));
    setFormWidget(CBouquetEditorForm::channels_in_bouquet, std::move(channels_in_bouquet),
    [=]{
        // updateViewValue()
        string title = asString(_form->value(CBouquetEditorForm::channels_in_bouquet)).toUTF8();
		int row = _form->channelInBouquetModelRow(title);
		channels_in_bouquet_ptr->setCurrentIndex(row);
    },
    [=]{
        // updateModelValue()
        string code = _form->channelsInBouquet(channels_in_bouquet_ptr->currentIndex()); 
		_form->setValue(CBouquetEditorForm::channels_in_bouquet, code);
    });

    auto channel_remove_button = bindWidget("channels-in-bouquet-remove-button", 
                                make_unique<WPushButton>("Entfernen"));
    channel_remove_button->clicked().connect(this, &CBouquetEditorView::removeChannel);

    auto dab_channels = make_unique<WSelectionBox>();
    auto _dab_channels_ptr = dab_channels.get();
    dab_channels->setModel(_form->getDabChannels());
    dab_channels->activated().connect(this, &CBouquetEditorView::dabChannelActivated);
    setFormWidget(CBouquetEditorForm::dab_channels, std::move(dab_channels),
    [=]{
        // updateViewValue()
        string title = asString(_form->value(CBouquetEditorForm::dab_channels)).toUTF8();
		int row = _form->dabChannelsModelRow(title);
		_dab_channels_ptr->setCurrentIndex(row);
    },
    [=]{
        // updateModelValue()
        string code = _form->dabChannels(_dab_channels_ptr->currentIndex()); 
		_form->setValue(CBouquetEditorForm::dab_channels, code);
    });
    //bindWidget("dab-channels", move(dab_channels));

    auto dab_channels_add_button = bindWidget("dab-channels-add-button", 
                                make_unique<WPushButton>("Hinzufügen"));
    dab_channels_add_button->clicked().connect(this, &CBouquetEditorView::addDabChannelToBouquet);

    // Web Channels Filter
    auto filter1 = make_unique<WComboBox>();
    filter1->setModel(_form->getFilter1());
    filter1->activated().connect(this, &CBouquetEditorView::updateFilter2);
    // connection between model and view and settings.xml
    auto filter1_ptr = filter1.get();
    setFormWidget(CBouquetEditorForm::filter_1, std::move(filter1),
    [=]{
        // updateViewValue()
        string title = asString(_form->value(CBouquetEditorForm::filter_1)).toUTF8();
		int row = _form->filter1ModelRow(title);
		filter1_ptr->setCurrentIndex(row);
    },
    [=]{
        // updateModelValue()
        string code = _form->filter1(filter1_ptr->currentIndex()); 
		_form->setValue(CBouquetEditorForm::filter_1, code);
    }); 

    auto filter2 = make_unique<WComboBox>();
    filter2->setModel(_form->getFilter2());
    filter2->activated().connect(this, &CBouquetEditorView::showChannelList);
    auto filter2_ptr = filter2.get();
    setFormWidget(CBouquetEditorForm::filter_2, std::move(filter2), 
    [=]{
        // updateViewValue()
        string title = asString(_form->value(CBouquetEditorForm::filter_2)).toUTF8();
		int row = _form->filter2ModelRow(title);
		filter2_ptr->setCurrentIndex(row);
    },
    [=]{
        // updateModelValue()
        string code = _form->filter2(filter2_ptr->currentIndex()); 
		_form->setValue(CBouquetEditorForm::filter_2, code);
    });

    auto web_channels = make_unique<WSelectionBox>();
    _web_channels_ptr = web_channels.get();
    //web_channels->setSelectionMode(Wt::SelectionMode::Extended);
    web_channels->setModel(_form->getWebChannels());
    web_channels->activated().connect(this, &CBouquetEditorView::webChannelActivated);
    setFormWidget(CBouquetEditorForm::web_channels, std::move(web_channels),
    [=]{
        // updateViewValue()
        string title = asString(_form->value(CBouquetEditorForm::web_channels)).toUTF8();
		int row = _form->webChannelsModelRow(title);
		_web_channels_ptr->setCurrentIndex(row);
    },
    [=]{
        // updateModelValue()
        string code = _form->webChannels(_web_channels_ptr->currentIndex());
		_form->setValue(CBouquetEditorForm::web_channels, code);
    });
    //bindWidget("web-channels", move(web_channels));

    auto web_channels_add_button = bindWidget("web-channels-add-button", 
                                make_unique<WPushButton>("Hinzufügen"));
    web_channels_add_button->clicked().connect(this, &CBouquetEditorView::addWebChannelToBouquet);

    auto station_name_line = make_unique<WLineEdit>();
    station_name_line->setWidth("260px");
    auto _station_name_line = station_name_line.get();
    station_name_line->keyWentUp().connect(this, [=]{
        _form->setValue(CBouquetEditorForm::station_name, _station_name_line->text());    
    });
    setFormWidget(CBouquetEditorForm::station_name, std::move(station_name_line));
    auto url_line = make_unique<WLineEdit>();
    url_line->setWidth("260px");
    auto _url_line = url_line.get();
    url_line->keyWentUp().connect(this, [=]{
        _form->setValue(CBouquetEditorForm::url, _url_line->text());    
    });
    setFormWidget(CBouquetEditorForm::url, std::move(url_line));
    auto url_line_logo = make_unique<WLineEdit>();
    url_line_logo->setWidth("260px");
    auto _url_line_logo = url_line_logo.get();
    url_line_logo->keyWentUp().connect(this, [=]{
        _form->setValue(CBouquetEditorForm::url_logo, _url_line_logo->text());    
    });
    setFormWidget(CBouquetEditorForm::url_logo, std::move(url_line_logo));
    auto add_station_button = bindWidget("add-station-button", 
                                make_unique<WPushButton>("Hinzufügen"));
    add_station_button->clicked().connect(this, &CBouquetEditorView::addNewChannelToBouquet);

    _form->setValue(CBouquetEditorForm::station_name, "Eigene Station");
    _form->setValue(CBouquetEditorForm::url, "http://Path-To-Eigene-Station.de");
    _form->setValue(CBouquetEditorForm::url_logo, "http://Path-To-his-logo-of-Eigene-Station.de");

    /* bindWidget("station_name", move(station_name_line));
    bindWidget("url", move(url_line));
    bindWidget("url-logo", move(url_line_logo)); */

    // Titles
    WString title = Wt::WString("Bouquet Editor");
    bindString("title-1", title);

    WString title2 = Wt::WString("Channels in Bouquet einfügen");
    bindString("title-2", title2);

    WString title3 = Wt::WString("Save or Cancel");
    bindString("title-3", title3);

    // Save and Cancel
    auto save_button = bindWidget("submit-button", 
                                make_unique<WPushButton>("Save"));
    save_button->clicked().connect(this, &CBouquetEditorView::saveBouquets);
    auto cancel_button = bindWidget("cancel-button", 
                                make_unique<WPushButton>("Cancel"));
    cancel_button->clicked().connect(this, &CBouquetEditorView::cancelBouquets);

    updateView(_form.get());
}

CBouquetEditorView::~CBouquetEditorView()
{

}

void CBouquetEditorView::dabChannelActivated(int idx)
{
    string code = _form->dabChannels(idx); 
	_form->setValue(CBouquetEditorForm::dab_channels, code);    
}

void CBouquetEditorView::webChannelActivated(int idx)
{
    string code = _form->webChannels(idx); 
	_form->setValue(CBouquetEditorForm::web_channels, code);
}

void CBouquetEditorView::newBouquet()
{
    WSelectionBox *bouquets = (WSelectionBox*) resolveWidget("bouquets");
    WLineEdit *bouquet_line = (WLineEdit*) resolveWidget("bouquet-name");
    WComboBox *bouquets_selection = (WComboBox*) resolveWidget("bouquet-selection");

    bouquets->addItem(bouquet_line->text());
    //bouquets_selection->addItem(bouquet_line->text());
    _form->addNewBouquet(asString(bouquet_line->text()).toUTF8());
}

void CBouquetEditorView::removeBouquet()
{
    WSelectionBox *bouquets = (WSelectionBox*) resolveWidget("bouquets");
    WComboBox *bouquets_selection = (WComboBox*) resolveWidget("bouquet-selection");

    _form->delBouquet(asString(bouquets->itemText(bouquets->currentIndex())).toUTF8());

    /* set<int> selection = bouquets->selectedIndexes();
    for (set<int>::iterator it = selection.begin(); it != selection.end(); ++it) 
    {
        cout << "Selected Index: " << *it << endl;
        bouquets->removeItem(*it);
        bouquets_selection->removeItem(*it);
        break;
    }*/
}

void CBouquetEditorView::removeChannel()
{
    WSelectionBox *channels_in_bouquet = (WSelectionBox*) resolveWidget("channels-in-bouquet");
    WComboBox *bouquets_selection = (WComboBox*) resolveWidget("bouquet-selection");

    _form->delChannel(asString(bouquets_selection->itemText(bouquets_selection->currentIndex())).toUTF8(), asString(channels_in_bouquet->itemText(channels_in_bouquet->currentIndex())).toUTF8());
}

void CBouquetEditorView::updateFilter2(int idxFilter1)
{
    //cerr << "Filter 1 möchte " << _form->filter1(idxFilter1) << " updaten." << endl;

    if (_form->filter1(idxFilter1) == "country")
        _rider_gui.getRadioServer().getSavedCountries();
    else if (_form->filter1(idxFilter1) == "all")
        _rider_gui.getRadioServer().getAllWebChannels();
    else if (_form->filter1(idxFilter1) == "countrycode")
        _rider_gui.getRadioServer().getSavedCountryCodeExact();
    else if (_form->filter1(idxFilter1) == "codec")
        _rider_gui.getRadioServer().getSavedCodecs();
    else if (_form->filter1(idxFilter1) == "language")
        _rider_gui.getRadioServer().getSavedLanguages();
    else if (_form->filter1(idxFilter1) == "tag")
        _rider_gui.getRadioServer().getSavedTags();
    else if (_form->filter1(idxFilter1) == "votes")
        _rider_gui.getRadioServer().getSavedChannelsOrderByVotes();
}

void CBouquetEditorView::showChannelList(int idxFilter2)
{
    //cerr << "idxFilter2 " << idxFilter2 << endl;
    //cerr << "Filter 2 möchte " << _form->filter2(idxFilter2) << " updaten." << endl;

    WComboBox *filter1 = (WComboBox*) resolveWidget("filter-1");

    if (filter1->itemText(filter1->currentIndex()) == "all" || 
        filter1->itemText(filter1->currentIndex()) == "country")
        _rider_gui.getRadioServer().getWebChannelsFromCountry(_form->filter2(idxFilter2));
    else if (filter1->itemText(filter1->currentIndex()) == "countrycode")
        _rider_gui.getRadioServer().getWebChannelsFromCountryCodeExact(_form->filter2(idxFilter2));
    else if (filter1->itemText(filter1->currentIndex()) == "codec")
        _rider_gui.getRadioServer().getWebChannelsFromCodec(_form->filter2(idxFilter2));
    else if (filter1->itemText(filter1->currentIndex()) == "language")
        _rider_gui.getRadioServer().getWebChannelsFromLanguage(_form->filter2(idxFilter2));
    else if (filter1->itemText(filter1->currentIndex()) == "tag")
        _rider_gui.getRadioServer().getWebChannelsFromTags(_form->filter2(idxFilter2));        
}

void CBouquetEditorView::showChannelsInBouquet(int idxBouquet)
{
    //cout << "Index: " << idxBouquet << endl;
    string code = _form->bouquetSelection(idxBouquet); 
    //cout << "Aktueller Index: " << code << endl;
	_form->setValue(CBouquetEditorForm::bouquet_selection, code);

    _form->actualizeChannelInBouquetModel(code);
}

void CBouquetEditorView::channelInBouquetClicked()
{
    string code = _form->bouquetSelection(0); 
	_form->setValue(CBouquetEditorForm::bouquet_selection, code);
}

void CBouquetEditorView::addDabChannelToBouquet()
{
    auto ci = _form->value(CBouquetEditorForm::dab_channels);
    string ch_str = asString(ci).toUTF8();
    
    CChannelItem *citem = (CChannelItem*) _form->getDabChannels()->item(_form->dabChannelsModelRow(ch_str), 0);
    
    auto bi = _form->value(CBouquetEditorForm::bouquet_selection);
    if (!bi.empty())
    {
        string b_str = asString(bi).toUTF8();

        string bouquet = asString(_form->getbouquetSelection()->item(_form->bouquetSelectionRowModel(b_str), 0)->text()).toUTF8();

        _form->addChannelToBouqet(bouquet, citem->getCurrentUUID(), citem->getChannelName());
    }
    else
    {
        auto wmb = make_unique<WMessageBox>("Error", "Please specify your bouquet!", Icon::Critical, StandardButton::Ok);
        wmb->show();
        WPushButton *ok = wmb->button(StandardButton::Ok);
        ok->clicked().connect(wmb.get(), &Wt::WDialog::accept);
        this->addChild(std::move(wmb));

        WApplication::instance()->triggerUpdate();
    }
}

void CBouquetEditorView::addWebChannelToBouquet()
{
    auto ci = _form->value(CBouquetEditorForm::web_channels);
    string ch_str = asString(ci).toUTF8();
    CChannelItem *citem = (CChannelItem*) _form->getWebChannels()->item(_form->webChannelsModelRow(ch_str), 0);

    auto bi = _form->value(CBouquetEditorForm::bouquet_selection);
    if (!bi.empty())
    {
        string b_str = asString(bi).toUTF8();

        string bouquet = asString(_form->getbouquetSelection()->item(_form->bouquetSelectionRowModel(b_str), 0)->text()).toUTF8();

        _form->addChannelToBouqet(bouquet, citem->getCurrentUUID(), citem->getChannelName());
    }
    else
    {
        auto wmb = make_unique<WMessageBox>("Error", "Please specify your bouquet!", Icon::Critical, StandardButton::Ok);
        wmb->show();
        WPushButton *ok = wmb->button(StandardButton::Ok);
        ok->clicked().connect(wmb.get(), &Wt::WDialog::accept);
        this->addChild(std::move(wmb));

        WApplication::instance()->triggerUpdate();
    }
}

void CBouquetEditorView::addNewChannelToBouquet()
{
    //updateModel(_form.get());

    auto station_name = _form->value(CBouquetEditorForm::station_name);
    string s_name = asString(station_name).toUTF8();
    auto url = _form->value(CBouquetEditorForm::url);
    string s_url = asString(url).toUTF8();
    auto url_logo = _form->value(CBouquetEditorForm::url_logo);
    string s_url_logo = asString(url_logo).toUTF8();

    auto bi = _form->value(CBouquetEditorForm::bouquet_selection);
    if (!bi.empty())
    {
        string b_str = asString(bi).toUTF8();

        string bouquet = asString(_form->getbouquetSelection()->item(_form->bouquetSelectionRowModel(b_str), 0)->text()).toUTF8();

        _form->addChannelToBouqet(bouquet, "-1;" + s_url + ";" + s_url_logo, s_name);
    }
    else
    {
        auto wmb = make_unique<WMessageBox>("Error", "Please specify your bouquet!", Icon::Critical, StandardButton::Ok);
        wmb->show();
        WPushButton *ok = wmb->button(StandardButton::Ok);
        ok->clicked().connect(wmb.get(), &Wt::WDialog::accept);
        this->addChild(std::move(wmb));

        WApplication::instance()->triggerUpdate();
    }
}

void CBouquetEditorView::saveBouquets()
{
    _rider_gui.getRadioServer().saveBouquetsInDB(_form->getAllBouquets());
}

void CBouquetEditorView::cancelBouquets()
{
    _rider_gui.cancelSettings();
}