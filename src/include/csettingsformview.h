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
#ifndef _C_SETTINGSFORMVIEW_H_
#define _C_SETTINGSFORMVIEW_H_

#include <Wt/WContainerWidget.h>
#include <Wt/WTemplateFormView.h>
#include <Wt/WLineEdit.h>
#include <Wt/WComboBox.h>
#include <Wt/WCheckBox.h>

#include "waverider-gui.h"
#include "csettingsform.h"

using namespace std;
using namespace Wt;

class WaveriderGUI;

class CSettingsFormView : public WTemplateFormView
{
    public:
        CSettingsFormView(WaveriderGUI& gui);
        virtual ~CSettingsFormView();

        // will be called from GUI over RadioServer
        void setFormData(string device, string ipaddress, string port, string recordPath, string radioBrowserURL, string donotreencode, string streaming, string streamingAddress, string streamingFormat, string recordFormat, string metadata, string saveMetadata);

    private:
        WaveriderGUI& _rider_gui;
        //WContainerWidget& _mainWidget;
        shared_ptr<CSettingsForm> form;     

        void process();
        void cancel();
};

#endif