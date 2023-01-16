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
#ifndef _C_DBOINTERNETCHANNEL_H_
#define _C_DBOINTERNETCHANNEL_H_

#include <iostream>

#include <Wt/Dbo/Dbo.h>
#include "cdbocountry.h"
#include "service.h"

namespace dbo = Wt::Dbo;

using namespace std;
using namespace Wt;

class CDBOCountry;
class CService;
class CDBOInternetChannel
{
    public:
        string stationuuid;
        string name;
        string url;
        string url_resolved;
        string homepage;
        string favicon;
        string tags;
        dbo::ptr<CDBOCountry> country;
        string state;
        string language;
        string languagecode;
        int votes;
        string codec;
        int bitrate;
        dbo::ptr<CService> service;
        
        CDBOInternetChannel();
        ~CDBOInternetChannel();

        template<class Action> void persist(Action& a)  
        {
            dbo::field(a, stationuuid, "stationuuid");
            dbo::field(a, name, "name");
            dbo::field(a, url, "url");    
            dbo::field(a, url_resolved, "url_resolved");
            dbo::field(a, homepage, "homepage");
            dbo::field(a, favicon, "favicon");
            dbo::field(a, tags, "tags");
            dbo::belongsTo(a, country, "country");
            dbo::field(a, state, "state");
            dbo::field(a, language, "language");        
            dbo::field(a, languagecode, "languagecode");
            dbo::field(a, votes, "votes");
            dbo::field(a, codec, "codec");
            dbo::field(a, bitrate, "bitrate");
            dbo::belongsTo(a, service);
        }  
};

#endif