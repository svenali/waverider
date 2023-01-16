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
#ifndef _C_DBOBOUQUET_H_
#define _C_DBOBOUQUET_H_

#include <iostream>

#include <Wt/Dbo/Dbo.h>

#include "cdbointernetchannel.h"
#include "service.h"

namespace dbo = Wt::Dbo;

using namespace std;
using namespace Wt;

class CDBOInternetChannel;
class CService;
class CDBOBouquet
{
    public:
        string name;
        dbo::collection< dbo::ptr<CService> > services;

        CDBOBouquet();
        ~CDBOBouquet();

        template<class Action> void persist(Action& a)
        {
            dbo::field(a, name, "name");
            dbo::hasMany(a, services, dbo::ManyToMany, "channel_belongsTo_bouquet");
        } 
};

#endif