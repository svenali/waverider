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
#ifndef _C_SETTINGS_H_
#define _C_SETTINGS_H_

#include <iostream>

#include <Wt/Dbo/Dbo.h>

namespace dbo = Wt::Dbo;

using namespace std;
using namespace Wt;

class CSettings
{
    public:
        CSettings();

        string _radioDevice;
        string _ipaddress;
        string _port;
        string _recordPath; 
        string _radioBrowserURL;

        string _doNotReEncodeIChannels;
        string _recordFormat;
        string _streaming;
        string _streamingAddress;
        string _streamingFormat;

        string _metadata;    // Retrieve Metadata
        string _saveMetadata;

        void setRadioDevice(string radioDevice) { _radioDevice = radioDevice; }
        void setIPAddress(string ipaddress) { _ipaddress = ipaddress; }
        void setPort(string port) { _port = port; }
        void setRecordPath(string recordPath) { _recordPath = recordPath; }
        void setRadioBrowserURL(string radioBrowserURL) { _radioBrowserURL = radioBrowserURL; }
        void setDoNotReEncodeIChannels(string reencoding) { _doNotReEncodeIChannels = reencoding; }
        void setRecordFormat(string format) { _recordFormat = format; }
        void setStreaming(string streaming) { _streaming = streaming; }
        void setStreamingAddress(string saddress) { _streamingAddress = saddress; }
        void setStreamingFormat(string sformat) { _streamingFormat = sformat; }
        void setRetrieveMetadata(string metadata) { _metadata = metadata; }
        void setSaveMetadata(string saveMetadata) { _saveMetadata = saveMetadata; }

        // must declare AND implement in this header file
        template<class Action> void persist(Action& a)
        {
            dbo::field(a, _radioDevice, "radioDevice");
            dbo::field(a, _ipaddress, "ipaddress");
            dbo::field(a, _port, "port"); 
            dbo::field(a, _recordPath, "recordPath");
            dbo::field(a, _radioBrowserURL, "radioBrowserURL");

            dbo::field(a, _doNotReEncodeIChannels, "doNotReEncodeIChannels");
            dbo::field(a, _recordFormat, "recordFormat");
            dbo::field(a, _streaming, "streaming");
            dbo::field(a, _streamingAddress, "streamingAddress");
            dbo::field(a, _streamingFormat, "streamingFormat");

            dbo::field(a, _metadata, "retrieveMetadata");
            dbo::field(a, _saveMetadata, "saveMetadata");
        }
};

#endif