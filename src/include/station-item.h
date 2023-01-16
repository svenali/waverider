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
#ifndef _C_STATION_ITEM_H_
#define _C_STATION_ITEM_H_

#include <Wt/WMenuItem.h>
#include <Wt/WWidget.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WText.h>

#include <iostream>

using namespace std;
using namespace Wt;

class CStationItem : public WMenuItem
{
    public:
        CStationItem(string name, unique_ptr< WWidget > contents=nullptr);
        CStationItem(uint32_t serviceID, string serviceName, string channelID, 
        unique_ptr< WWidget > contents=nullptr);
        CStationItem(string serviceName, string url, unique_ptr<WWidget> contents);
        ~CStationItem();

        // dab+
        uint32_t getServiceID() { return _serviceID; };
        string getChannelID() { return _channelID; };
        string getServiceName() { return _serviceName; };
        string getServiceIDASHexString();

        // web
        string getBroadcastType() { return _type; };
        string getWebURL() { return _url; };

        Signal<CStationItem*>& channelClicked() { return _newStation; };
        Signal<CStationItem*> _newStation; 

    private:
        void stationLinkClicked();

        // dab+
        uint32_t _serviceID;
        string _serviceName;
        string _channelID;

        // web
        string _type;
        string _url;
};

#endif