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
#ifndef _C_SIMPLE_TIMER_H_
#define _C_SIMPLE_TIMER_H_

#include <iostream>       // std::cout
#include <thread>         // std::thread

using namespace std;

class CSimpleTimer
{
    bool clear = false;

public:
    template<typename Function> void setTimeout(Function function, int delay);
    template<typename Function> void setInterval(Function function, int interval);
    inline void stopTimer();
};

template<typename Function> void CSimpleTimer::setTimeout(Function function, int delay) 
{
    this->clear = false;
    thread t([=]() 
    {
        if(this->clear) return;
        this_thread::sleep_for(chrono::milliseconds(delay));
        if(this->clear) return;
        function();
    });
    t.detach();
}

template<typename Function> void CSimpleTimer::setInterval(Function function, int interval) 
{
    this->clear = false;
    thread t([=]() 
    {
        while(true) 
        {
            if(this->clear) return;
            this_thread::sleep_for(chrono::milliseconds(interval));
            if(this->clear) return;
            function();
        }
    });
    t.detach();
}

void CSimpleTimer::stopTimer() 
{
    this->clear = true;
}

#endif