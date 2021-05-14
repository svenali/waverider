#include "csettingsform.h"

const CSettingsForm::DeviceMap CSettingsForm::devices = 
{
    {"rtl-tcp", {"Netzwerkkommunikation"}},
    {"rtl-sdr", {"USB-Stick mit RTL Chip"}}
};

// Zuordnung zu Begriffen der settings.xml
const WFormModel::Field CSettingsForm::DeviceField = "device";  
const WFormModel::Field CSettingsForm::IPAddress = "address";
const WFormModel::Field CSettingsForm::Port = "port";
const WFormModel::Field CSettingsForm::RecordPath = "recordpath";

CSettingsForm::CSettingsForm()
    :   WFormModel()
{
    _deviceModel = make_shared<WStandardItemModel>(devices.size()+1, 1);
    _deviceModel->setData(0, 0, string(" "), ItemDataRole::Display);
    _deviceModel->setData(0, 0, string(), ItemDataRole::User);
    
    int row = 1;
    for (DeviceMap::const_iterator i = devices.begin(); i != devices.end(); ++i) 
    {
        _deviceModel->setData(row, 0, i->first, ItemDataRole::Display);
        _deviceModel->setData(row++, 0, i->first, ItemDataRole::User);
    }

    addField(DeviceField);
    addField(IPAddress);
    addField(Port);
    addField(RecordPath);

    setValue(DeviceField, string("rtl-sdr"));
    setValue(IPAddress, string("0.0.0.0"));
    setValue(Port, string("1234"));
    setValue(RecordPath, string("undefined"));
}

CSettingsForm::~CSettingsForm()
{

}

string CSettingsForm::deviceCode (int row) 
{
    cout << "DeviceCode angefordert" << endl;

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

void CSettingsForm::updateRecordPath(const string& path)
{
    _recordPath = string(path);
}