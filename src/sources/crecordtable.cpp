/*
 *    Copyright (C) 2024
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

#include "crecordtable.h"
#include <boost/algorithm/string.hpp>

CRecordTable::CRecordTable():WTable(),
_rowCount(0), 
_columnCount(0)
{

}

CRecordTable::~CRecordTable()
{

}

void CRecordTable::sortRecords(int column, bool asc)
{
    cout << "Sort by header in sortRecords ..." << endl;

    datamodel_t data = this->exportData(column);
    
    // Sortiere den Vektor nach den Schlüsseln
    switch (column)
    {
        case 0:
        case 3:
            std::sort(data.begin(), data.end(), 
                [=](const pair<string, row_t>& a, const pair<string, row_t>& b) -> bool
                {
                    /* cout << "a.first: " << a.first << endl;
                    cout << "b.first: " << b.first << endl;
                    cout << stoi(a.first) << " < " << stoi(b.first) << " is: " << (stoi(a.first) < stoi(b.first)) << endl; */
                    if (asc)
                        return stoi(a.first) < stoi(b.first);
                    else
                        return stoi(a.first) > stoi(b.first);
                }
            );
            break;
        case 1:
            std::sort(data.begin(), data.end(), 
                [=](const pair<string, row_t>& a, const pair<string, row_t>& b) -> bool
                {
                    if (asc)
                        return a.first < b.first;
                    else
                        return a.first > b.first;
                }
            );
        break;
        case 2:
            std::sort(data.begin(), data.end(), 
                [=](const pair<string, row_t>& a, const pair<string, row_t>& b) -> bool
                {
                    vector<string> datas_a;
                    boost::split(datas_a, a.first, boost::is_any_of("-"));
                    int year_a = stoi(datas_a[2]);
                    int month_a = stoi(datas_a[1]);
                    int day_a = stoi(datas_a[0]);
                    WDate a_date(year_a, month_a, day_a); 

                    vector<string> datas_b;
                    boost::split(datas_b, b.first, boost::is_any_of("-"));
                    int year_b = stoi(datas_b[2]);
                    int month_b = stoi(datas_b[1]);
                    int day_b = stoi(datas_b[0]);
                    WDate b_date(year_b, month_b, day_b); 

                    if (asc)
                        return a_date < b_date;
                    else
                        return a_date > b_date;
                }
            );
            break;
        default:
            break;
    }

    int row = 1;
    
    // Gib die sortierten Daten aus
    for (const auto& pair : data) 
    {
        this->elementAt(row, 0)->clear();
        this->elementAt(row, 0)->addNew<WText>(pair.second.row);
        this->elementAt(row, 1)->clear();
        auto resource = make_shared<WFileResource>(pair.second.url);
        resource->setMimeType("audio/wav");
        resource->suggestFileName(pair.second.url_text);
        this->elementAt(row, 1)->addNew<WAnchor>(WLink(resource), pair.second.url_text);
        this->elementAt(row, 2)->clear();
        this->elementAt(row, 2)->addNew<WText>(pair.second.date);
        this->elementAt(row, 3)->clear();
        this->elementAt(row, 3)->addNew<WText>(pair.second.size);
        this->elementAt(row, 4)->clear();
        auto c = this->elementAt(row, 4)->addNew<CDeletePushButton>("Content", pair.second.url_text);
        //c->deleteClicked().connect(this, &CRecords::showContent);
        auto b = this->elementAt(row, 4)->addNew<CDeletePushButton>("Delete", pair.second.url);
        //b->deleteClicked().connect(this, &CRecords::deleteFile);
        
        row++;
    }
}

datamodel_t CRecordTable::exportData(int column)
{
    datamodel_t data;

    for (int i = 1; i < this->rowCount(); i++)
    {
        row_t row;
        WText* row_text = dynamic_cast<WText*>(this->elementAt(i, 0)->widget(0));
        row.row = row_text->text().toUTF8();
        WAnchor* anchor = dynamic_cast<WAnchor*>(this->elementAt(i, 1)->widget(0));
        row.url = anchor->link().url();
        row.url_text = anchor->text().toUTF8();
        WText* date_text = dynamic_cast<WText*>(this->elementAt(i, 2)->widget(0));
        row.date = date_text->text().toUTF8();
        WText* size_text = dynamic_cast<WText*>(this->elementAt(i, 3)->widget(0));
        row.size = size_text->text().toUTF8();
    
        switch(column)
        {
            case 0:
                data.push_back(pair<string, row_t>(row.row, row));
                break;
            case 1:
                data.push_back(pair<string, row_t>(row.url_text, row));
                break;
            case 2:
                data.push_back(pair<string, row_t>(row.date, row));
                break;
            case 3:
                data.push_back(pair<string, row_t>(row.size, row));
                break;
            default:
                data.push_back(pair<string, row_t>(row.row, row));
                break;
        }
    }

    return data;
}