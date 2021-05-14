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
#ifndef _CDELETEPUSHBUTTON_H_
#define _CDELETEPUSHBUTTON_H_

#include <Wt/WPushButton.h>
#include <Wt/WString.h>

#include <string>

using namespace Wt;
using namespace std;

class CDeletePushButton : public WPushButton 
{
    public:
        CDeletePushButton(const WString &text, string file);
        ~CDeletePushButton();

        string fileToDelete() { return _file; }

        Signal<CDeletePushButton*>& deleteClicked() { return _deleteFile; };
        Signal<CDeletePushButton*> _deleteFile; 

    private:
        string _file;
        void deleteFileClicked();
};

#endif //_CDELETEPUSHBUTTON_H_ 