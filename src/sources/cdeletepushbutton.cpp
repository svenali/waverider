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
#include "cdeletepushbutton.h"

CDeletePushButton::CDeletePushButton(const WString &text, string file)
    :   WPushButton(text),
        _file(file)
{
    this->clicked().connect(this, &CDeletePushButton::deleteFileClicked);   
}

CDeletePushButton::~CDeletePushButton()
{

}

void CDeletePushButton::deleteFileClicked()
{
    _deleteFile.emit(this);
}

