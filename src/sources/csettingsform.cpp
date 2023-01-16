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
#include "csettingsform.h"

const CSettingsForm::DeviceMap CSettingsForm::devices = 
{
    {"rtl-tcp", {"Netzwerkkommunikation"}},
    {"rtl-sdr", {"USB-Stick mit RTL Chip"}}
};

const CSettingsForm::StreamingFormatMap CSettingsForm::streamingFormats =
{
    {"AAC", {"Advanced Audio Codec / m4a Container"}},
    {"ALAC", {"Apple Lossless Audio Codec / m4a Container"}},
    {"FLAC", {"Free Lossless Audio Codec / ogg Container"}},
    {"MP2", {"MPEG 1 Layer 2 Audio Frames"}},
    {"MP3", {"MPEG 1 Layer 3 Audio Frames"}},
    {"VORBIS", {"ogg/Vorbis Open Source Audio Codec"}},
    {"WAV", {"WAVE/pure PCM Uncompressed Audio Data"}}
};

const CSettingsForm::RecordFormatMap CSettingsForm::recordFormats =
{
    {"AAC", {"Advanced Audio Codec (*.aac)"}},
    {"ALAC", {"Apple Lossless Audio Codec (*.m4a)"}},
    {"FLAC", {"Free Lossless Audio Codec (*.flac)"}},
    {"MP2", {"MPEG 1 Layer 2 Audio Frames (*.mp2)"}},
    {"MP3", {"MPEG 1 Layer 3 Audio Frames (*.mp3)"}},
    {"VORBIS", {"ogg/Vorbis Open Source Audio Codec (*.ogg)"}},
    {"WAV", {"WAVE/pure PCM Uncompressed Audio Data (*.wav)"}}
};

// Zuordnung zu Begriffen der settings.xml
const WFormModel::Field CSettingsForm::DeviceField = "device";  
const WFormModel::Field CSettingsForm::IPAddress = "address";
const WFormModel::Field CSettingsForm::Port = "port";
const WFormModel::Field CSettingsForm::RecordPath = "recordpath";
const WFormModel::Field CSettingsForm::RadioBrowserURL = "radiobrowserurl";
const WFormModel::Field CSettingsForm::DoNotReEncodeIChannels = "donot-reencode-ichannels";
const WFormModel::Field CSettingsForm::RecordFormat = "format";
const WFormModel::Field CSettingsForm::Streaming = "streaming";
const WFormModel::Field CSettingsForm::StreamingAddress = "streaming-address";
const WFormModel::Field CSettingsForm::StreamingFormat = "streaming-format";
const WFormModel::Field CSettingsForm::RetrieveMetadata = "metadata";
const WFormModel::Field CSettingsForm::SaveMetadata = "save-metadata";

CSettingsForm::CSettingsForm()
    :   WFormModel()
{
    _deviceModel = make_shared<WStandardItemModel>(devices.size()+1, 1);
    _deviceModel->setData(0, 0, string(" "), ItemDataRole::Display);
    _deviceModel->setData(0, 0, string(), ItemDataRole::Display);
    
    int row = 1;
    for (DeviceMap::const_iterator i = devices.begin(); i != devices.end(); ++i) 
    {
        _deviceModel->setData(row, 0, i->first, ItemDataRole::Display);
        _deviceModel->setData(row++, 0, i->first, ItemDataRole::User);
    }

    row = 0;
    _streamingFormatModel = make_shared<WStandardItemModel>(streamingFormats.size(), 1);
    for (StreamingFormatMap::const_iterator i = streamingFormats.begin(); i != streamingFormats.end(); ++i) 
    {
        _streamingFormatModel->setData(row, 0, i->first, ItemDataRole::Display);
        _streamingFormatModel->setData(row++, 0, i->first, ItemDataRole::User);
    }

    row = 0;
    _recordFormatModel = make_shared<WStandardItemModel>(recordFormats.size(), 1);
    for (RecordFormatMap::const_iterator i = recordFormats.begin(); i != recordFormats.end(); ++i) 
    {
        _recordFormatModel->setData(row, 0, i->first, ItemDataRole::Display);
        _recordFormatModel->setData(row++, 0, i->first, ItemDataRole::User);
    }

    addField(DeviceField);
    addField(IPAddress);
    addField(Port);
    addField(RecordPath);
    addField(RadioBrowserURL);

    addField(DoNotReEncodeIChannels);
    addField(RecordFormat);
    addField(StreamingAddress);
    addField(StreamingFormat);

    addField(RetrieveMetadata);
    addField(SaveMetadata);

    setValue(DeviceField, string("rtl-sdr"));
    setValue(IPAddress, string("0.0.0.0"));
    setValue(Port, string("1234"));
    setValue(RecordPath, string("undefined"));
    setValue(RadioBrowserURL, string(""));

    setValue(DoNotReEncodeIChannels, "true");
    setValue(RecordFormat, string("FLAC"));
    setValue(StreamingAddress, string("/streaming"));
    setValue(StreamingFormat, string("FLAC"));

    setValue(RetrieveMetadata, string("true"));
    setValue(SaveMetadata, string("true"));
}

CSettingsForm::~CSettingsForm()
{

}

string CSettingsForm::deviceCode (int row) 
{
    return asString(_deviceModel->data(row, 0, ItemDataRole::User)).toUTF8();
}

void CSettingsForm::updateDeviceModel(const string& deviceCode) 
{
    /* _deviceModel->clear();

    DeviceMap::const_iterator i = devices.find(deviceCode);

    if (i != devices.end()) 
    {
        const vector<string>& device = i->second;

        // The initial text shown in the city combo box should be an empty
        // string.
        _deviceModel->appendRow(make_unique<Wt::WStandardItem>());

        for (unsigned j = 0; j < device.size(); ++j)
        {
            _deviceModel->appendRow(make_unique<Wt::WStandardItem>(device[j]));
        }
    } 
    else 
    {
        _deviceModel->appendRow(make_unique<Wt::WStandardItem>("(Wähle zuerst das Device)"));
    }*/
}

int CSettingsForm::deviceModelRow(const string& code)
{
    for (int i = 0; i < _deviceModel->rowCount(); ++i)
	    if (deviceCode(i) == code)
	        return i;

	return -1;
}

string CSettingsForm::streamingFormat(int row)
{
    return asString(_streamingFormatModel->data(row, 0, ItemDataRole::User)).toUTF8();
}

int CSettingsForm::streamingFormatModelRow(const string& code)
{
    for (int i = 0; i < _streamingFormatModel->rowCount(); ++i)
	    if (streamingFormat(i) == code)
	        return i;

	return -1;
}

string CSettingsForm::recordFormat(int row)
{
    return asString(_streamingFormatModel->data(row, 0, ItemDataRole::User)).toUTF8();
}

int CSettingsForm::recordFormatModelRow(const string& code)
{
    for (int i = 0; i < _streamingFormatModel->rowCount(); ++i)
	    if (recordFormat(i) == code)
	        return i;

	return -1;
}

void CSettingsForm::updateRecordPath(const string& path)
{
    _recordPath = string(path);
}

void CSettingsForm::updateRadioBrowserURL(const string& url)
{
    _radioBrowserURL = string(url);
}

void CSettingsForm::updateDoNotReEncode(string r)
{
    _doNotReEncodeIChannels = r;
}
        
void CSettingsForm::updateStreaming(string r)
{
    _streaming = r;
}

void CSettingsForm::updateStreamingAddress(const string& address)
{
    _streamingAddress = string(address);
}

void CSettingsForm::updateMetadata(string md)
{
    _metadata = md;
}

void CSettingsForm::updateSaveMetadata(string smd)
{
    _saveMetadata = smd;
}