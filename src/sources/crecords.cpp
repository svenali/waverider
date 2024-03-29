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
    _files->addStyleClass("files-widget col-sm-12 col-xs-12 col-md-12 col-lg-12");
    _files->setHeight("390");

    _filecontent = this->addNew<WContainerWidget>();
    _filecontent->addStyleClass("files-widget col-sm-12 col-xs-12 col-md-12 col-lg-12");
    _filecontent->addNew<WText>("<h3>Content of File: </h3>");
    _filecontent->setHeight("390");

    _ContentTable = nullptr;
}

CRecords::~CRecords()
{

}

int CRecords::dirExists(const char* const path)
{
    struct stat info;

    int statRC = stat( path, &info );
    if( statRC != 0 )
    {
        if (errno == ENOENT)  { return 0; } // something along the path does not exist
        if (errno == ENOTDIR) { return 0; } // something in path prefix is not a dir
        return -1;
    }

    return ( info.st_mode & S_IFDIR ) ? 1 : 0;
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
    table_->elementAt(0, 4)->addNew<WText>("Actions");

    int row = 1;

    DIR *directory;
    struct dirent *file;
    string path = _rider_gui.recordPath();
    bool pathExists = (path != "undefined" && dirExists(path.c_str()) != -1 && dirExists(path.c_str()) != 0) ? true : false;
    if (pathExists)
    {
        directory = opendir(_rider_gui.recordPath().c_str());

        while (file = readdir(directory)) 
        {
            string filename_temp(file->d_name);
            string path_to_file = _rider_gui.recordPath() + "/" + filename_temp;
            if (filename_temp != "." && filename_temp != "..")
            {
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
                auto c = table->elementAt(row, 4)->addNew<CDeletePushButton>("Content", filename_temp);
                c->deleteClicked().connect(this, &CRecords::showContent);
                auto b = table->elementAt(row, 4)->addNew<CDeletePushButton>("Delete", path_to_file);
                b->deleteClicked().connect(this, &CRecords::deleteFile);

                row++;
            }
        }
    }

    _files->addWidget(std::move(table));
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

void CRecords::showContent(WPushButton *c)
{
    CDeletePushButton* d = (CDeletePushButton*) c;
    string file(d->fileToDelete());
    cerr << file << endl;

    _rider_gui.getRadioServer().showContentOfRecordedFile(file);
}

void CRecords::initContentPage(vector<vector<string>> c)
{
    if (_ContentTable != nullptr)
    {
        _filecontent->removeWidget(_ContentTable);
        _ContentTable = nullptr;
    }

    auto table = make_unique<WTable>();
    _ContentTable = table.get();
    table->setHeaderCount(1);
    table->setWidth(Wt::WLength("100%"));

    _ContentTable->elementAt(0, 0)->addNew<WText>("#");
    _ContentTable->elementAt(0, 1)->addNew<WText>("Time");
    _ContentTable->elementAt(0, 2)->addNew<WText>("Title");
    _ContentTable->elementAt(0, 3)->addNew<WText>("Artist");
    _ContentTable->elementAt(0, 4)->addNew<WText>("Cover");
    _ContentTable->elementAt(0, 4)->setWidth(WLength("150px"));

    int row = 1;

    for (auto it = begin(c); it != end(c); ++it)
    {
        vector<string> content = *it;

        _ContentTable->elementAt(row, 0)->addNew<WText>(to_string(row));
        _ContentTable->elementAt(row, 1)->addNew<WText>(content[0]);
        _ContentTable->elementAt(row, 2)->addNew<WText>(content[1]);
        _ContentTable->elementAt(row, 3)->addNew<WText>(content[2]);
        auto image = make_unique<WImage>(content[3]);
        image->setWidth("150px");
        _ContentTable->elementAt(row, 4)->addWidget(std::move(image));
        _ContentTable->elementAt(row, 4)->setWidth(WLength("150px"));

        row++;
    }

    _filecontent->addWidget(std::move(table));
}