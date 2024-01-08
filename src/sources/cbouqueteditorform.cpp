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
#include "cbouqueteditorform.h"

const CBouquetEditorForm::FilterMap CBouquetEditorForm::_standardFilter =
{
    { "", "1" },
    { "all", "2" },
    { "country", "3" },
    { "countrycode", "4" },
    { "language", "5" },
    { "tag", "6" },
    { "codec", "7" },
    { "votes", "8" }
};

// Zuordnung zu Begriffen der settings.xml
const WFormModel::Field CBouquetEditorForm::bouquet_name = "bouquet-name";  
const WFormModel::Field CBouquetEditorForm::bouquets = "bouquets";
const WFormModel::Field CBouquetEditorForm::bouquet_selection = "bouquet-selection";
const WFormModel::Field CBouquetEditorForm::filter_1 = "filter-1";
const WFormModel::Field CBouquetEditorForm::filter_2 = "filter-2";
const WFormModel::Field CBouquetEditorForm::channels_in_bouquet = "channels-in-bouquet";
const WFormModel::Field CBouquetEditorForm::dab_channels = "dab-channels";
const WFormModel::Field CBouquetEditorForm::web_channels = "web-channels";
const WFormModel::Field CBouquetEditorForm::station_name = "station_name";
const WFormModel::Field CBouquetEditorForm::url = "url";
const WFormModel::Field CBouquetEditorForm::url_logo = "url-logo";

CBouquetEditorForm::CBouquetEditorForm():WFormModel()
{
    _bouquets = make_shared<WStandardItemModel>();
    _bouquets_selection = make_shared<WStandardItemModel>();
    
    _filter_1 = make_shared<WStandardItemModel>(_standardFilter.size(), 1);
    int row = 0;
    for (FilterMap::const_iterator i = _standardFilter.begin(); i != _standardFilter.end(); ++i) 
    {
        _filter_1->setData(row, 0, i->first, ItemDataRole::Display);
        _filter_1->setData(row++, 0, i->first, ItemDataRole::User);
    }

    _filter_2 = make_shared<WStandardItemModel>();
    _channels_in_bouquet = make_shared<WStandardItemModel>();
    _dab_channels = make_shared<WStandardItemModel>();
    _web_channels = make_shared<WStandardItemModel>();

    addField(bouquet_name);
    addField(bouquets);
    addField(bouquet_selection);
    addField(filter_1);
    addField(filter_2);
    addField(channels_in_bouquet);
    addField(dab_channels);
    addField(web_channels);
    addField(station_name);
    addField(url);
    addField(url_logo);
}

CBouquetEditorForm::~CBouquetEditorForm()
{
    
}

void CBouquetEditorForm::setBouquetName(string bouquetName)
{

}
        
string CBouquetEditorForm::bouquet(int row)
{
    return asString(_bouquets->item(row,0)->text()).toUTF8();
}

int CBouquetEditorForm::bouquetRowModel(string title)
{
    for (int i = 0; i < _bouquets->rowCount(); ++i)
    {
	    if (bouquet(i) == title)
	        return i;
    }

	return -1;
}
        
string CBouquetEditorForm::bouquetSelection(int row)
{
    return asString(_bouquets_selection->item(row,0)->text()).toUTF8();
}

int CBouquetEditorForm::bouquetSelectionRowModel(string title)
{
    for (int i = 0; i < _bouquets_selection->rowCount(); ++i)
    {
	    if (bouquetSelection(i) == title)
	        return i;
    }

	return -1;
}

int CBouquetEditorForm::filter1ModelRow(string title)
{
    for (int i = 0; i < _filter_1->rowCount(); ++i)
    {
	    if (filter1(i) == title)
	        return i;
    }

	return -1;
}

int CBouquetEditorForm::filter2ModelRow(string title)
{
    for (int i = 0; i < _filter_1->rowCount(); ++i)
    {
	    if (filter2(i) == title)
	        return i;
    }

	return -1;
}

string CBouquetEditorForm::filter1(int row)
{
    return asString(_filter_1->data(row, 0, ItemDataRole::User)).toUTF8();
}

string CBouquetEditorForm::filter2(int row)
{
    //return asString(_filter_2->data(row, 0, ItemDataRole::User)).toUTF8();
    if (_filter_2->rowCount() == 0)
        return "";

    return asString(_filter_2->item(row,0)->text()).toUTF8();
}

void CBouquetEditorForm::updateFilter2(const vector<string> items)
{
    _filter_2->clear();
    int row = 0;
    for (auto it=begin(items); it != end(items); ++it)
    {
        auto newItem = make_unique<WStandardItem>();
        newItem->setText(*it); 
        _filter_2->appendRow(std::move(newItem));
    }
}

void CBouquetEditorForm::updateWebChannels(const map<string, string> items)
{
    _web_channels->clear();
    for (auto it = begin(items); it != end(items); ++it)
    {
        auto newItem = make_unique<CChannelItem>();
        newItem->setData(it->first, it->second);
        _web_channels->appendRow(std::move(newItem));
    }
}

void CBouquetEditorForm::updateDabChannels(const map<string, string> items)
{
    _dab_channels->clear();
    for (auto it = begin(items); it != end(items); ++it)
    {
        auto newItem = make_unique<CChannelItem>();
        newItem->setData(it->first, it->second);
        _dab_channels->appendRow(std::move(newItem));
    }    
}

string CBouquetEditorForm::channelsInBouquet(int row)
{
    return asString(_channels_in_bouquet->item(row,0)->text()).toUTF8();
}

int CBouquetEditorForm::channelInBouquetModelRow(string title)
{
    for (int i = 0; i < _channels_in_bouquet->rowCount(); ++i)
    {
	    if (channelsInBouquet(i) == title)
	        return i;
    }

	return -1;
}

int CBouquetEditorForm::dabChannelsModelRow(string title)
{
    for (int i = 0; i < _dab_channels->rowCount(); ++i)
    {
	    if (dabChannels(i) == title)
	        return i;
    }

	return -1;
}

string CBouquetEditorForm::dabChannels(int row)
{
    if (_dab_channels->rowCount() == 0)
        return "";

    return asString(_dab_channels->item(row,0)->text()).toUTF8();
}

string CBouquetEditorForm::webChannels(int row)
{
    if (_web_channels->rowCount() == 0)
        return "";

    return asString(_web_channels->item(row,0)->text()).toUTF8();
}

int CBouquetEditorForm::webChannelsModelRow(string title)
{
    for (int i = 0; i < _web_channels->rowCount(); ++i)
    {
	    if (webChannels(i) == title)
	        return i;
    }

	return -1;
}

void CBouquetEditorForm::setStationName(string stationName)
{

}

void CBouquetEditorForm::setURL(string url)
{

}

void CBouquetEditorForm::setURLLogo(string urllogo)
{

}

void CBouquetEditorForm::addNewBouquet(string bouquet_name)
{
    map<string, string> channels = _channelToBouquetMap[bouquet_name];
    _channelToBouquetMap[bouquet_name] = channels;

    auto standard_item = make_unique<WStandardItem>();
    standard_item->setText(bouquet_name);
    _bouquets_selection->appendRow(std::move(standard_item));
}

void CBouquetEditorForm::delBouquet(string bouquet_name)
{
    auto it = _channelToBouquetMap.find(bouquet_name);
    _channelToBouquetMap.erase(it);

    for (int i = 0; i < _bouquets_selection->rowCount(); ++i)
    {
	    if (bouquetSelection(i) == bouquet_name)
        {
            _bouquets_selection->removeRow(i);
        }
    }

    for (int i = 0; i < _bouquets->rowCount(); ++i)
    {
	    if (bouquet(i) == bouquet_name)
        {
            _bouquets->removeRow(i);
        }
    }
}

void CBouquetEditorForm::delChannel(string bouquet_name, string name)
{
    auto it = _channelToBouquetMap.find(bouquet_name);
    
    // Channel suchen
    string uuid;
    for (auto it2 = begin(it->second); it2 != end(it->second); ++it2)
    {
        if (it2->second == name)
        {
            uuid = it2->first;
        }   
    }

    // delete in map for server site
    auto del_it = it->second.find(uuid);
    it->second.erase(del_it);

    // delete channel in model
    for (int i = 0; i < _channels_in_bouquet->rowCount(); ++i)
    {
	    if (channelsInBouquet(i) == name)
        {
            _channels_in_bouquet->removeRow(i);
        }
    }
}

bool CBouquetEditorForm::addChannelToBouqet(string bouquet_name, string uuid, string name)
{
    map<string, string> channels = _channelToBouquetMap[bouquet_name];
    channels[uuid] = name;
    _channelToBouquetMap[bouquet_name] = channels;

    if (this->channelInBouquetModelRow(name) == -1)
    {
        auto newItem = make_unique<CChannelItem>();
        newItem->setData(uuid, name);
        _channels_in_bouquet->appendRow(std::move(newItem));
    }
}

bool CBouquetEditorForm::actualizeChannelInBouquetModel(string bouquet_name)
{
    _channels_in_bouquet->clear();

    map<string, string> channels = _channelToBouquetMap[bouquet_name];

    for (auto it = begin(channels); it != end(channels); ++it)
    {
        auto newItem = make_unique<CChannelItem>();
        newItem->setData(it->first, it->second);
        _channels_in_bouquet->appendRow(std::move(newItem));
    }
}

void CBouquetEditorForm::loadBouquetsFromDB(const ChannelToBouquetMap b)
{
    //_channelToBouquetMap = b;

    int idx = 0;

    for (auto it = begin(b); it != end(b); ++it)
    {
        auto standard_item = make_unique<WStandardItem>();
        standard_item->setText(it->first);
        _bouquets_selection->appendRow(std::move(standard_item));
        
        auto standard_item_2 = make_unique<WStandardItem>();
        standard_item_2->setText(it->first);
        _bouquets->appendRow(std::move(standard_item_2));

        _channelToBouquetMap[it->first] = it->second;
        
        if (idx == 0)
        {
            map<string, string> channels = it->second;

            for (auto it2 = begin(channels); it2 != end(channels); ++it2)
            {
                auto newItem = make_unique<CChannelItem>();
                newItem->setData(it2->first, it2->second);
                _channels_in_bouquet->appendRow(std::move(newItem));
            }
        }

        idx++;
    }
}