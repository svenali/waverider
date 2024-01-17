/*
 *    Copyright (C) 2022
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
#ifndef _CRECORDS_H_
#define _CRECORDS_H_

#include <Wt/WContainerWidget.h>
#include <Wt/WLabel.h>
#include <Wt/WFileResource.h>
#include <Wt/WLink.h>
#include <Wt/WAnchor.h>
#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WText.h>
#include <Wt/WMessageBox.h>

#include <dirent.h>
#include <stdio.h>
#include <cstring>

#include <sys/stat.h>
#include <filesystem>

#include "waverider-gui.h"
#include "cdeletepushbutton.h"
#include "crecordtable.h"

using namespace Wt;
using namespace std;

class WaveriderGUI;

typedef enum SortBy
{
    HEADER,
    DATE,
    SIZE,
    HASHTAG_NUMBER,
    NOTHING
} sortby_t;

class CRecords : public WContainerWidget
{
    public:
        CRecords(WaveriderGUI &gui);
        ~CRecords();

        void initDownloadPage();
        void deleteFile(WPushButton *b);
        void showContent(WPushButton *c);
        void updateDownloadPage();
        void initContentPage(vector<vector<string>> c);

        void sortByHeader();
        void sortByDate();
        void sortBySize();
        void sortByHashTagNumber();

    private:
        int dirExists(const char* const path);

        WaveriderGUI &_rider_gui;
        WContainerWidget *_files;
        WContainerWidget *_filecontent;

        WTable * _ContentTable;
        sortby_t _lastSorted;
};

#endif //_CRECORDS_H_