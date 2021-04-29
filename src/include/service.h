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
#ifndef _C_SERVICE_H_
#define _C_SERVICE_H_

#include <iostream>

#include <Wt/Dbo/Dbo.h>

namespace dbo = Wt::Dbo;

using namespace std;

class CService
{
    public:
        CService();

        string _serviceID;
        string _label;
        string _currentChannel;  

        // must declare AND implement in this header file
        template<class Action> void persist(Action& a)
        {
            dbo::field(a, _serviceID, "serviceID");
            dbo::field(a, _label, "Label");
            dbo::field(a, _currentChannel, "Channel"); 
        }
};

#endif