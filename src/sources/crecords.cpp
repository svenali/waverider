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
#include "crecords.h"

CRecords::CRecords(WaveriderGUI &gui)
    :   WContainerWidget(),
        _rider_gui(gui)
{
    this->setStyleClass("row");

    _files = this->addNew<WContainerWidget>();
    _files->addStyleClass("col-sm-12 col-xs-12 col-md-12 col-lg-12");
}

CRecords::~CRecords()
{

}

void CRecords::initDownloadPage()
{
    auto label = _files->addNew<WLabel>();
    label->setText("<h3>Content of the directory: " + _rider_gui.recordPath() + "</h3>");

    auto table = make_unique<WTable>();
    auto table_ = table.get();
    table->setHeaderCount(1);
    table->setWidth(Wt::WLength("100%"));

    table_->elementAt(0, 0)->addNew<WText>("#");
    table_->elementAt(0, 1)->addNew<WText>("Filename");
    table_->elementAt(0, 2)->addNew<WText>("Date");
    table_->elementAt(0, 3)->addNew<WText>("Size");
    table_->elementAt(0, 4)->addNew<WText>("Action");

    int row = 1;

    DIR *directory;
    struct dirent *file;
    directory = opendir(_rider_gui.recordPath().c_str());
    while (file = readdir(directory)) 
    {
        string filename_temp(file->d_name);
        string path_to_file = _rider_gui.recordPath() + "/" + filename_temp;
        if (filename_temp != "." && filename_temp != "..")
        {
            cout << "Create Anchor for File " << filename_temp << endl;

            struct stat st;
            stat(path_to_file.c_str(), &st);
            int size = st.st_size;
            char date[30];
            strftime(date, 20, "%d-%m-%y", localtime(&(st.st_ctime)));

            auto resource = make_shared<WFileResource>(_rider_gui.recordPath()+"/"+filename_temp);
            resource->setMimeType("audio/wav");
            resource->suggestFileName(filename_temp);
            table->elementAt(row, 0)->addNew<WText>(WString("{1}").arg(row));
            table->elementAt(row, 1)->addNew<WAnchor>(WLink(resource), filename_temp);
            table->elementAt(row, 2)->addNew<WText>(WString(date));
            table->elementAt(row, 3)->addNew<WText>(WString("{1}").arg(size));
            auto b = table->elementAt(row, 4)->addNew<CDeletePushButton>("Delete", path_to_file);
            b->deleteClicked().connect(this, &CRecords::deleteFile);

            row++;
        }
    }

    _files->addWidget(move(table));
}

void CRecords::updateDownloadPage()
{
    _files->clear();
    initDownloadPage();
}

void CRecords::deleteFile(WPushButton *b)
{
    CDeletePushButton* d = (CDeletePushButton*) b;
    string deleteFileName(d->fileToDelete());

    StandardButton result = 
        WMessageBox::show("Confirm", "Do you really want to delete the file " + 
                            d->fileToDelete() + " Continue ?",
                             StandardButton::Yes | StandardButton::No);

    if (result == StandardButton::Yes)
    {
        if (remove(deleteFileName.c_str()) != 0)
        {
            cerr << "Error CRecords: File can not be removed." << endl;
        }
        else
        {
            cout << "CRecords: File deleted." << endl;

            updateDownloadPage();
        }
    }
}