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
        static const Field RadioBrowserURL;

        static const Field DoNotReEncodeIChannels;
        static const Field RecordFormat;
        static const Field Streaming;
        static const Field StreamingAddress;
        static const Field StreamingFormat;

        static const Field RetrieveMetadata;
        static const Field SaveMetadata;

        CSettingsForm();
        ~CSettingsForm();

        shared_ptr<WAbstractItemModel> deviceModel() { return _deviceModel; }
        shared_ptr<WAbstractItemModel> streamingFormatModel() { return _streamingFormatModel; }
        shared_ptr<WAbstractItemModel> recordFormatModel() { return _recordFormatModel; }
        string deviceCode(int row);
        int deviceModelRow(const string& code);
        void updateDeviceModel(const string& deviceCode);
        string recordPath() { return _recordPath; }
        void updateRecordPath(const string& path);
        string radioBrowserURL() { return _radioBrowserURL; }
        void updateRadioBrowserURL(const string& url);
        
        string streamingFormat(int row);
        int streamingFormatModelRow(const string& code);
        string recordFormat(int row);
        int recordFormatModelRow(const string& code);
        
        void updateDoNotReEncode(string r);
        string doNotReEncode() { return _doNotReEncodeIChannels; }
        void updateStreaming(string r);
        string streaming() { return _streaming; }
        void updateStreamingAddress(const string& address);
        string streamingAddress() { return _streamingAddress; }

        string metadata() { return _metadata; };
        void updateMetadata(string md);
        string saveMetadata() { return _saveMetadata; }
        void updateSaveMetadata(string smd);

        typedef map< string, vector<string> > DeviceMap;
        typedef map< string, vector<string> > StreamingFormatMap;
        typedef map< string, vector<string> > RecordFormatMap;

    private:
        static const DeviceMap devices;
        static const StreamingFormatMap streamingFormats;
        static const RecordFormatMap recordFormats;

        shared_ptr<WStandardItemModel> _deviceModel;
        shared_ptr<WStandardItemModel> _streamingFormatModel;
        shared_ptr<WStandardItemModel> _recordFormatModel;

        string _recordPath;
        string _radioBrowserURL;

        string _streamingAddress;
        string _doNotReEncodeIChannels;
        string _streaming;

        string _metadata;
        string _saveMetadata;
};

#endif