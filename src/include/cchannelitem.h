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
#ifndef _CCHANNELITEM_H
#define _CCHANNELITEM_H

#include <Wt/WStandardItem.h>

using namespace std;
using namespace Wt;

class CChannelItem : public WStandardItem
{
    public:
        CChannelItem();
        ~CChannelItem();

        void setData(string uuid, string channel_name);

        string getCurrentUUID() { return _uuid; }; 
        string getChannelName() { return _name; };

    private:
        string _uuid;
        string _name;
};

#endif //_CCHANNELSELECTIONBOX_H