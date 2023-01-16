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
#ifndef _C_BOUQUETEDITOR_FORM_H_
#define _C_BOUQUETEDITOR_FORM_H_

#include <map>
#include <string>
#include <Wt/WFormModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WAbstractItemModel.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WModelIndex.h>

#include "cchannelitem.h"

using namespace Wt;
using namespace std;

class CBouquetEditorForm : public WFormModel
{
    public:
        static const Field bouquet_name;
        static const Field bouquets;
        static const Field bouquet_selection;
        static const Field filter_1;
        static const Field filter_2;
        static const Field channels_in_bouquet;
        static const Field dab_channels;
        static const Field web_channels;

        // add individual channel
        static const Field station_name;
        static const Field url;
        static const Field url_logo;

        CBouquetEditorForm();
        ~CBouquetEditorForm();

        void setBouquetName(string bouquetName);
        string getBouquetName() { return _bouquet_name; };
        string bouquet(int row);
        int bouquetRowModel(string title);
        shared_ptr<WStandardItemModel> getbouquet() { return _bouquets; };
        string bouquetSelection(int row);
        shared_ptr<WStandardItemModel> getbouquetSelection() { return _bouquets_selection; };
        int bouquetSelectionRowModel(string title);
        string filter1(int row);
        int filter1ModelRow(string title);
        shared_ptr<WAbstractItemModel> getFilter1() { return _filter_1; };
        string filter2(int row);
        int filter2ModelRow(string title);
        shared_ptr<WAbstractItemModel> getFilter2() { return _filter_2; };
        void updateFilter2(const vector<string> items);
        string channelsInBouquet(int row);
        shared_ptr<WStandardItemModel> getChannelsInBouquet() { return _channels_in_bouquet; };
        int channelInBouquetModelRow(string title);
        string dabChannels(int row);
        int dabChannelsModelRow(string title);
        shared_ptr<WStandardItemModel> getDabChannels() { return _dab_channels; };
        void updateDabChannels(const map<string, string> items);
        string webChannels(int row);
        int webChannelsModelRow(string title);
        shared_ptr<WStandardItemModel> getWebChannels() { return _web_channels; };
        void updateWebChannels(const map<string, string> items);
        void setStationName(string stationName);
        string getStationName() { return _station_name; };
        void setURL(string url);
        string getURL() { return _url; };
        void setURLLogo(string urllogo);
        string getURLLogo() { return _url_logo; };

        void addNewBouquet(string bouquet_name);
        void delBouquet(string bouquet_name);
        void delChannel(string bouquet_name, string name);
        bool addChannelToBouqet(string bouquet_name, string uuid, string name);
        bool actualizeChannelInBouquetModel(string bouquet_name);
        
        typedef map< string, string > FilterMap;
        typedef map< string, map<string, string>> ChannelToBouquetMap;

        ChannelToBouquetMap getAllBouquets() { return _channelToBouquetMap; };
        void loadBouquetsFromDB(const ChannelToBouquetMap b);

    private:
        string _bouquet_name;
        shared_ptr<WStandardItemModel> _bouquets;
        shared_ptr<WStandardItemModel> _bouquets_selection;
        static const FilterMap _standardFilter;
        shared_ptr<WStandardItemModel> _filter_1;
        shared_ptr<WStandardItemModel> _filter_2;
        shared_ptr<WStandardItemModel> _channels_in_bouquet;
        shared_ptr<WStandardItemModel> _dab_channels;
        shared_ptr<WStandardItemModel> _web_channels;
        string _station_name;
        string _url;
        string _url_logo;
        ChannelToBouquetMap _channelToBouquetMap;
};

#endif