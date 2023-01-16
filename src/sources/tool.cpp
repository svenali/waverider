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
#include "tool.h"
#include <boost/algorithm/string.hpp>

map<string, string> _russianUpperLower;

void init_tools()
{
    _russianUpperLower["А"] = "а";
    _russianUpperLower["Б"] = "б";
    _russianUpperLower["В"] = "в";
    _russianUpperLower["Г"] = "г";
    _russianUpperLower["Д"] = "д";
    _russianUpperLower["Е"] = "е";
    _russianUpperLower["Ё"] = "ё";
    _russianUpperLower["З"] = "з";
    _russianUpperLower["Ж"] = "ж";
    _russianUpperLower["И"] = "и";
    _russianUpperLower["Ы"] = "ы";
    _russianUpperLower["Й"] = "й";
    _russianUpperLower["К"] = "к";
    _russianUpperLower["Л"] = "л";
    _russianUpperLower["М"] = "м";
    _russianUpperLower["Н"] = "н";
    _russianUpperLower["О"] = "о";
    _russianUpperLower["П"] = "п";
    _russianUpperLower["Р"] = "р";
    _russianUpperLower["С"] = "с";
    _russianUpperLower["Т"] = "т";
    _russianUpperLower["У"] = "у";
    _russianUpperLower["Ф"] = "ф";
    _russianUpperLower["Х"] = "х";
    _russianUpperLower["Ц"] = "ц";
    _russianUpperLower["Ч"] = "ч";
    _russianUpperLower["Ш"] = "ш";
    _russianUpperLower["Щ"] = "щ";
    _russianUpperLower["Э"] = "э";
    _russianUpperLower["Ю"] = "ю";
    _russianUpperLower["Я"] = "я";
    _russianUpperLower["а"] = "а";
    _russianUpperLower["б"] = "б";
    _russianUpperLower["в"] = "в";
    _russianUpperLower["г"] = "г";
    _russianUpperLower["д"] = "д";
    _russianUpperLower["е"] = "е";
    _russianUpperLower["ё"] = "ё";
    _russianUpperLower["з"] = "з";
    _russianUpperLower["ж"] = "ж";
    _russianUpperLower["и"] = "и";
    _russianUpperLower["й"] = "й";
    _russianUpperLower["ы"] = "ы";
    _russianUpperLower["к"] = "к";
    _russianUpperLower["л"] = "л";
    _russianUpperLower["м"] = "м";
    _russianUpperLower["н"] = "н";
    _russianUpperLower["о"] = "о";
    _russianUpperLower["п"] = "п";
    _russianUpperLower["р"] = "р";
    _russianUpperLower["с"] = "с";
    _russianUpperLower["т"] = "т";
    _russianUpperLower["у"] = "у";
    _russianUpperLower["ф"] = "ф";
    _russianUpperLower["х"] = "х";
    _russianUpperLower["ц"] = "ц";
    _russianUpperLower["ч"] = "ч";
    _russianUpperLower["ш"] = "ш";
    _russianUpperLower["щ"] = "щ";
    _russianUpperLower["э"] = "э";
    _russianUpperLower["ю"] = "ю";
    _russianUpperLower["я"] = "я";
    _russianUpperLower[" "] = " ";   
    _russianUpperLower[","] = ",";
    _russianUpperLower[";"] = ";";
    _russianUpperLower["&"] = "&";
    _russianUpperLower[":"] = ":";
    _russianUpperLower["-"] = "-";
    _russianUpperLower["?"] = "?";
    _russianUpperLower["!"] = "!";
}

bool to_bool(string expr)
{
    if (expr.find("true") != string::npos || expr.find("True") != string::npos || expr.find("TRUE") != string::npos)
        return true;
    else   
        return false;
}

bool isRussian(string s)
{
    for (int i = 0; i < s.length(); i+=2)
    {
        try
        {
            string cs = s.substr(i, 2);
            string r = _russianUpperLower.at(cs);
            
            if (i > 4)
            {
                break;
            }
        }
        catch(const std::out_of_range& e)
        {
            //if this happens, then string is not russian
            return false;
        }
    }

    return true;
}

string string_to_lower(string s)
{
    if (isRussian(s))
    {
        string new_string = "";
        string low = s;
        boost::trim(low);
        cerr << "Russian Language found and: " << low.length() << " is string length." << endl;
        for (int i = 0; i < low.length(); i+=2)
        {
            try
            {
                string cs = low.substr(i, 2);
                string r = _russianUpperLower.at(cs);
                
                new_string += r;
            }
            catch(const std::exception& e)
            {
                new_string += " ";
                
                cerr << "Oi: " << low.substr(i,1) << endl;
                cerr << e.what() << endl; 

                i--;
            }
        }

        return new_string;
    }
    else
    {
        return boost::to_lower_copy(s);
    }
}

int levinstein(string name1, string name2)
{
    int i,j,l1,l2,t,track;
    int dist[50][50];

    // input two c strings
    string ss1 = string_to_lower(name1);
    string ss2 = string_to_lower(name2);
    boost::trim(ss1);
    boost::trim(ss2);

    cerr << "Name 1 (1): " << ss1 << " (1) " << name1 << endl;
    cerr << "Name 2 (1): " << ss2 << " (2) " << name2 << endl;
    
    const char* s1 = ss1.c_str();
    const char* s2 = ss2.c_str();

    //stores the lenght of strings s1 and s2
    l1 = name1.length();
    l2 = name2.length();

    for(i=0; i <= l1; i++) 
    {
        dist[0][i] = i;
    }

    for(j=0; j<=l2; j++) 
    {
        dist[j][0] = j;
    }

    for (j=1; j<=l1; j++) 
    {
        for(i=1; i<=l2; i++) 
        {
            if(s1[i-1] == s2[j-1]) 
            {
                track = 0;
            } 
            else 
            {
                track = 1;
            }
            t = MIN((dist[i-1][j]+1),(dist[i][j-1]+1));
            dist[i][j] = MIN(t,(dist[i-1][j-1]+track));
        }
    }
    cout << "The Levinstein distance is:" << dist[l2][l1] << endl;
    
    return dist[l2][l1];
}