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

#ifndef _CRECORDTABLE_H_
#define _CRECORDTABLE_H_

#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WText.h>
#include <Wt/WAnchor.h>
#include <Wt/WFileResource.h>
#include <Wt/WDate.h>
#include <cdeletepushbutton.h>
#include <crecords.h>

using namespace Wt;
using namespace std;

typedef struct
{
    string row;
    string url;
    string url_text;
    string date;
    string size;
} row_t;
typedef vector<pair<string, row_t>> datamodel_t;

class CRecordTable : public WTable
{
    public:
        CRecordTable();
        ~CRecordTable();

        void sortRecords(int column, bool asc = true);

    private:
        datamodel_t _data;
        datamodel_t exportData(int column);

        int _rowCount;
        int _columnCount;
};

#endif // _CRECORDTABLE_H_