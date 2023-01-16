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
#ifndef _TOOL_
#define _TOOL_

#include <string>
#include <iostream>
#include <map>

using namespace std;

#define MIN(x,y) ((x) < (y) ? (x) : (y)) //calculate minimum between two values

void init_tools();

bool to_bool(string expr);

// Levinstein Algorithm for messuring distance between two strings
int levinstein(string name1, string name2);

// Specials for Russian Alphabet
extern map<string, string> _russianUpperLower;
bool isRussian(string s);
string string_to_lower(string s);

#endif