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
#ifndef _CBOUQUETEDITOR_H
#define _CBOUQUETEDITOR_H

#include <Wt/WContainerWidget.h>
#include <Wt/WTemplateFormView.h>
#include <Wt/WSelectionBox.h>

#include "cbouqueteditorform.h"
#include "waverider-gui.h"

using namespace std;
using namespace Wt;

class WaveriderGUI;

class CBouquetEditorView : public WTemplateFormView
{
    public:
        CBouquetEditorView(WaveriderGUI& gui);
        ~CBouquetEditorView();

        shared_ptr<CBouquetEditorForm> getFormModel() { return _form; };

    private:
        void newBouquet();
        void removeBouquet();
        void removeChannel();
        void webChannelActivated(int idx);
        void dabChannelActivated(int idx);
        void addWebChannelToBouquet();
        void addDabChannelToBouquet();
        void addNewChannelToBouquet();

        void updateFilter2(int idxFilter1);
        void showChannelList(int idxFilter2);
        void showChannelsInBouquet(int idxBouquet);
        void channelInBouquetClicked();
        void saveBouquets();
        void cancelBouquets();

        WSelectionBox *_web_channels_ptr;
        shared_ptr<CBouquetEditorForm> _form;
        WaveriderGUI& _rider_gui; 
};

#endif