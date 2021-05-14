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
#ifndef _C_SETTINGS_FORM_H_
#define _C_SETTINGS_FORM_H_

#include <map>
#include <string>
#include <Wt/WFormModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WAbstractItemModel.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WModelIndex.h>

using namespace Wt;
using namespace std;

class CSettingsForm : public WFormModel
{
    public:
        static const Field DeviceField;
        static const Field IPAddress;
        static const Field Port;
        static const Field RecordPath;

        CSettingsForm();
        ~CSettingsForm();

        shared_ptr<WAbstractItemModel> deviceModel() { return _deviceModel; }
        string deviceCode (int row);
        int deviceModelRow(const string& code);
        void updateDeviceModel(const string& deviceCode);
        string recordPath() { return _recordPath; }
        void updateRecordPath(const string& path);

        typedef map< string, vector<string> > DeviceMap;

    private:
        static const DeviceMap devices;
        shared_ptr<WStandardItemModel> _deviceModel;
        string _recordPath;
};

#endif