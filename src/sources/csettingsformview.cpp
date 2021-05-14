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
#include "csettingsformview.h"

CSettingsFormView::CSettingsFormView(WaveriderGUI& gui)
    :   WTemplateFormView(),
        _rider_gui(gui)
        //_mainWidget(mainWidget)
{
    form = make_shared<CSettingsForm>();

    setTemplateText(tr("settings"));
    addFunction("id", &WTemplate::Functions::id);
    addFunction("block", &WTemplate::Functions::id);

    auto deviceCB = make_unique<WComboBox>();
    auto deviceCB_ = deviceCB.get();
    deviceCB->setModel(form->deviceModel());

    deviceCB_->activated().connect([=] {
	    string code = form->deviceCode(deviceCB_->currentIndex());
        cout << "Code: " << code << endl;
	    //form->updateDeviceModel(code);
        if (code == "rtl-tcp") 
        {
            setCondition("if-register", true);
        }
        else
        {
            setCondition("if-register", false);
        }
	});

    setFormWidget(CSettingsForm::DeviceField, move(deviceCB),
        [=] { 
            // updateViewValue()
            string code = asString(form->value(CSettingsForm::DeviceField)).toUTF8();
		    int row = form->deviceModelRow(code);
		    deviceCB_->setCurrentIndex(row);
            if (code == "rtl-tcp") 
            {
                setCondition("if-register", true);
            }
            else
            {
                setCondition("if-register", false);
            }
	    },

        [=] { // updateModelValue()
            string code = form->deviceCode(deviceCB_->currentIndex());
		    form->setValue(CSettingsForm::DeviceField, code);
        }
    );

    auto port_line = make_unique<WLineEdit>();
    port_line->setWidth("60px");
    auto recordPath = make_unique<WLineEdit>();
    auto recordPath_ = recordPath.get();
    //recordPath->changed().connect(this, &CSettings::update);
    setFormWidget(CSettingsForm::IPAddress, make_unique<WLineEdit>());
    setFormWidget(CSettingsForm::Port, move(port_line));
    setFormWidget(CSettingsForm::RecordPath, move(recordPath));

    WString title = Wt::WString("Settings");
    bindString("title", title);

    auto button = bindWidget("submit-button", make_unique<WPushButton>("Save"));
    button->clicked().connect(this, &CSettingsFormView::process);

    auto cancel = bindWidget("cancel-button", make_unique<WPushButton>("Cancel"));
    cancel->clicked().connect(this, &CSettingsFormView::cancel);

    updateView(form.get());
}

CSettingsFormView::~CSettingsFormView()
{

}

void CSettingsFormView::setFormData(string device, string ipaddress, string port, string recordPath)
{
    form->setValue(CSettingsForm::DeviceField, device);
    form->setValue(CSettingsForm::IPAddress, ipaddress);
    form->setValue(CSettingsForm::Port, port);
    form->setValue(CSettingsForm::RecordPath, recordPath);

    updateView(form.get());
}

void CSettingsFormView::process()
{
    updateModel(form.get());

    cout << "Device: " << asString(form->value(CSettingsForm::DeviceField)).toUTF8() << " ausgewählt." << endl;
    cout << "RecordPath: " << asString(form->value(CSettingsForm::RecordPath)).toUTF8() << " ." << endl;

    _rider_gui.getRadioServer().saveSettings(
        asString(form->value(CSettingsForm::DeviceField)).toUTF8(),
        asString(form->value(CSettingsForm::IPAddress)).toUTF8(),
        asString(form->value(CSettingsForm::Port)).toUTF8(),
        asString(form->value(CSettingsForm::RecordPath)).toUTF8()
    );
}

void CSettingsFormView::cancel()
{
    _rider_gui.cancelSettings();
}