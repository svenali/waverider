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
#include "station-item.h"

CStationItem::CStationItem(string name, unique_ptr< WWidget > contents)
: WMenuItem(name, move(contents)),
  _serviceID(0x1),
  _serviceName(name),
  _channelID("0XX"),
  _type("bouquet"),
  _url("")
{
    /* auto serviceStationWidget = make_unique<WContainerWidget>();
    serviceStationWidget->setStyleClass("row");
    
    auto infoContainer = make_unique<WContainerWidget>();
    infoContainer->setStyleClass("container-fluid");
    
    auto urlString = make_unique<WText>(this->getWebURL());
    urlString->setStyleClass("col-xs-12 col-md-12 col-lg-12");
    infoContainer->addWidget(move(urlString));

    serviceStationWidget->addStyleClass("serviceStation-web");
    serviceStationWidget->addWidget(move(infoContainer)); */

    this->clicked().connect(this, &CStationItem::stationLinkClicked);

    //this->addWidget(move(serviceStationWidget)); 
}

CStationItem::CStationItem(uint32_t serviceID, string serviceName, string channelID, unique_ptr<WWidget> contents)
: WMenuItem(serviceName, move(contents)),
  _serviceID(serviceID),
  _serviceName(serviceName),
  _channelID(channelID),
  _url(""),
  _type("dab+")
{
    auto serviceStationWidget = make_unique<WContainerWidget>();
    serviceStationWidget->setStyleClass("row");
    
    auto infoContainer = make_unique<WContainerWidget>();
    infoContainer->setStyleClass("container-fluid");
    
    auto hexString = make_unique<WText>(this->getServiceIDASHexString());
    hexString->setStyleClass("col-xs-4 col-md-4 col-lg-4");
    infoContainer->addWidget(std::move(hexString));

    auto channelIDString = make_unique<WText>(channelID);
    channelIDString->setStyleClass("col-xs-8 col-md-8 col-lg-8");
    infoContainer->addWidget(std::move(channelIDString));

    serviceStationWidget->addStyleClass("serviceStation");
    serviceStationWidget->addWidget(std::move(infoContainer));

    this->clicked().connect(this, &CStationItem::stationLinkClicked);

    this->addWidget(std::move(serviceStationWidget));
}

CStationItem::CStationItem(string serviceName, string url, unique_ptr<WWidget> contents)
: WMenuItem(serviceName, move(contents)),
  _serviceID(0x0),
  _serviceName(serviceName),
  _channelID("00X"),
  _type("web"),
  _url(url)
{
    auto serviceStationWidget = make_unique<WContainerWidget>();
    serviceStationWidget->setStyleClass("row");
    
    auto infoContainer = make_unique<WContainerWidget>();
    infoContainer->setStyleClass("container-fluid");
    
    auto urlString = make_unique<WText>(this->getWebURL());
    urlString->setStyleClass("col-xs-12 col-md-12 col-lg-12");
    infoContainer->addWidget(std::move(urlString));

    /* auto channelIDString = make_unique<WText>(channelID);
    channelIDString->setStyleClass("col-xs-8 col-md-8 col-lg-8");
    infoContainer->addWidget(move(channelIDString)); */

    serviceStationWidget->addStyleClass("serviceStation-web");
    serviceStationWidget->addWidget(std::move(infoContainer));

    this->clicked().connect(this, &CStationItem::stationLinkClicked);

    this->addWidget(std::move(serviceStationWidget));
}

CStationItem::~CStationItem()
{

}

void CStationItem::stationLinkClicked()
{
    _newStation.emit(this);
}

string CStationItem::getServiceIDASHexString() 
{
    stringstream strstream;
    strstream << hex << _serviceID;
    return strstream.str();
}