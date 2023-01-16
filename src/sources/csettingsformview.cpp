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
    auto radioBrowserURL = make_unique<WLineEdit>();

    //recordPath->changed().connect(this, &CSettings::update);
    setFormWidget(CSettingsForm::IPAddress, make_unique<WLineEdit>());
    setFormWidget(CSettingsForm::Port, move(port_line));
    setFormWidget(CSettingsForm::RecordPath, move(recordPath));
    setFormWidget(CSettingsForm::RadioBrowserURL, move(radioBrowserURL));

    auto donotreencode = make_unique<WCheckBox>();
    auto donotreencode_ = donotreencode.get();
    setFormWidget(CSettingsForm::DoNotReEncodeIChannels, move(donotreencode),
        [=] { 
            // updateViewValue()
            string state = asString(form->value(CSettingsForm::DoNotReEncodeIChannels)).toUTF8();
            
		    if (state.compare("true") == 0)
            {
                donotreencode_->setChecked(true);
            }
            else
            {
                donotreencode_->setChecked(false);
            }
	    },

        [=] { // updateModelValue()
            if (donotreencode_->isChecked())
            {
                form->setValue(CSettingsForm::DoNotReEncodeIChannels, "true");
            }
            else
            {
                form->setValue(CSettingsForm::DoNotReEncodeIChannels, "false");
            }
        }
    );

    auto metadata = make_unique<WCheckBox>();
    auto metadata_ = metadata.get();
    setFormWidget(CSettingsForm::RetrieveMetadata, move(metadata),
        [=] { 
            // updateViewValue()
            string state = asString(form->value(CSettingsForm::RetrieveMetadata)).toUTF8();
            
		    if (state.compare("true") == 0)
            {
                metadata_->setChecked(true);
            }
            else
            {
                metadata_->setChecked(false);
            }
	    },

        [=] { // updateModelValue()
            if (metadata_->isChecked())
            {
                form->setValue(CSettingsForm::RetrieveMetadata, "true");
            }
            else
            {
                form->setValue(CSettingsForm::RetrieveMetadata, "false");
            }
        }
    );

    auto saveMetadata = make_unique<WCheckBox>();
    auto saveMetadata_ = saveMetadata.get();
    setFormWidget(CSettingsForm::SaveMetadata, move(saveMetadata),
        [=] { 
            // updateViewValue()
            string state = asString(form->value(CSettingsForm::SaveMetadata)).toUTF8();
            
		    if (state.compare("true") == 0)
            {
                saveMetadata_->setChecked(true);
            }
            else
            {
                saveMetadata_->setChecked(false);
            }
	    },

        [=] { // updateModelValue()
            if (saveMetadata_->isChecked())
            {
                form->setValue(CSettingsForm::SaveMetadata, "true");
            }
            else
            {
                form->setValue(CSettingsForm::SaveMetadata, "false");
            }
        }
    );

    auto streaming = make_unique<WCheckBox>();
    auto streaming_ = streaming.get();
    streaming_->clicked().connect([=] {
	    if (streaming_->isChecked()) 
        {
            setCondition("streaming-condition", true);
        }
        else
        {
            setCondition("streaming-condition", false);
        }
	});

    setFormWidget(CSettingsForm::Streaming, move(streaming),
        [=] { 
            // updateViewValue()
            string state = asString(form->value(CSettingsForm::Streaming)).toUTF8();
            if (state.compare("true") == 0)
            {
                streaming_->setChecked(true);
                setCondition("streaming-condition", true);
            }
            else
            {
                streaming_->setChecked(false);
                setCondition("streaming-condition", false);
            }
	    },

        [=] { // updateModelValue()
            if (streaming_->isChecked())
            {
                form->setValue(CSettingsForm::Streaming, "true");
                setCondition("streaming-condition", true);
            }
            else
            {
                form->setValue(CSettingsForm::Streaming, "false");
                setCondition("streaming-condition", false);
            }
        }
    );

    auto streamingFormat  = make_unique<WComboBox>();
    auto streamingFormat_ = streamingFormat.get();
    streamingFormat->setModel(form->streamingFormatModel());

    setFormWidget(CSettingsForm::StreamingFormat, move(streamingFormat),
        [=] { 
            // updateViewValue()
            string code = asString(form->value(CSettingsForm::StreamingFormat)).toUTF8();
		    int row = form->streamingFormatModelRow(code);
		    streamingFormat_->setCurrentIndex(row);
	    },

        [=] { // updateModelValue()
            string code = form->streamingFormat(streamingFormat_->currentIndex());
		    form->setValue(CSettingsForm::StreamingFormat, code);
        }
    );

    auto recordFormat  = make_unique<WComboBox>();
    auto recordFormat_ = recordFormat.get();
    recordFormat->setModel(form->recordFormatModel());

    setFormWidget(CSettingsForm::RecordFormat, move(recordFormat),
        [=] { 
            // updateViewValue()
            string code = asString(form->value(CSettingsForm::RecordFormat)).toUTF8();
		    int row = form->recordFormatModelRow(code);
		    recordFormat_->setCurrentIndex(row);
	    },

        [=] { // updateModelValue()
            string code = form->recordFormat(recordFormat_->currentIndex());
		    form->setValue(CSettingsForm::RecordFormat, code);
        }
    );

    auto streamingAddress  = make_unique<WLineEdit>();
    auto streamingAddress_ = streamingAddress.get();
    setFormWidget(CSettingsForm::StreamingAddress, move(streamingAddress));

    WString title = Wt::WString("Settings");
    bindString("title", title);

    WString title_streaming = Wt::WString("Streaming");
    bindString("title-export", title_streaming);

    WString title_recording = Wt::WString("Recording");
    bindString("title-save", title_recording);

    auto button = bindWidget("submit-button", make_unique<WPushButton>("Save"));
    button->clicked().connect(this, &CSettingsFormView::process);

    auto cancel = bindWidget("cancel-button", make_unique<WPushButton>("Cancel"));
    cancel->clicked().connect(this, &CSettingsFormView::cancel);

    updateView(form.get());
}

CSettingsFormView::~CSettingsFormView()
{

}

void CSettingsFormView::setFormData(string device, string ipaddress, string port, string recordPath, string radioBrowserURL, string donotreencode, string streaming, string streamingAddress, string streamingFormat, string recordFormat, string metadata, string saveMetadata)
{
    form->setValue(CSettingsForm::DeviceField, device);
    form->setValue(CSettingsForm::IPAddress, ipaddress);
    form->setValue(CSettingsForm::Port, port);
    form->setValue(CSettingsForm::RecordPath, recordPath);
    form->setValue(CSettingsForm::RadioBrowserURL, radioBrowserURL);
    form->setValue(CSettingsForm::DoNotReEncodeIChannels, donotreencode);
    form->setValue(CSettingsForm::Streaming, streaming);
    form->setValue(CSettingsForm::StreamingAddress, streamingAddress);
    form->setValue(CSettingsForm::StreamingFormat, streamingFormat);
    form->setValue(CSettingsForm::RecordFormat, recordFormat);
    form->setValue(CSettingsForm::RetrieveMetadata, metadata);
    form->setValue(CSettingsForm::SaveMetadata, saveMetadata);

    updateView(form.get());
}

void CSettingsFormView::process()
{
    updateModel(form.get());

    _rider_gui.getRadioServer().saveSettings(
        asString(form->value(CSettingsForm::DeviceField)).toUTF8(),
        asString(form->value(CSettingsForm::IPAddress)).toUTF8(),
        asString(form->value(CSettingsForm::Port)).toUTF8(),
        asString(form->value(CSettingsForm::RecordPath)).toUTF8(),
        asString(form->value(CSettingsForm::RadioBrowserURL)).toUTF8(),
        asString(form->value(CSettingsForm::DoNotReEncodeIChannels)).toUTF8(),
        asString(form->value(CSettingsForm::Streaming)).toUTF8(),
        asString(form->value(CSettingsForm::StreamingAddress)).toUTF8(),
        asString(form->value(CSettingsForm::StreamingFormat)).toUTF8(),
        asString(form->value(CSettingsForm::RecordFormat)).toUTF8(),
        asString(form->value(CSettingsForm::RetrieveMetadata)).toUTF8(),
        asString(form->value(CSettingsForm::SaveMetadata)).toUTF8()
    );
}

void CSettingsFormView::cancel()
{
    _rider_gui.cancelSettings();
}