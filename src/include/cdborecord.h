/*
 *    Copyright (C) 2022
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
#ifndef _C_DBORECORD_H_
#define _C_DBORECORD_H_

#include <iostream>

#include <Wt/Dbo/Dbo.h>

namespace dbo = Wt::Dbo;

using namespace std;
using namespace Wt;

class CDBORecord
{
    public:
        string filename;
        chrono::system_clock::time_point timestamp;
        string titlename;
        string artist;
        vector<unsigned char> cover;
        int type;
        
        CDBORecord();
        ~CDBORecord();

        template<class Action> void persist(Action& a)  
        {
            dbo::field(a, filename, "filename");
            dbo::field(a, timestamp, "timestamp");
            dbo::field(a, titlename, "titlename");    
            dbo::field(a, artist, "artist");
            dbo::field(a, cover, "cover");
            dbo::field(a, type, "pic_type");
        }  
};

#endif