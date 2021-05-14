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
#include "waverider.h"

WaveriderApplication::WaveriderApplication(const WEnvironment& env, CRadioServer& radio_server)
    :   WApplication(env)
{
    setTitle("Waverider DAB+ Digital Radio");
    //setCssTheme("polished");  // First Try
    enableUpdates(true);

    useStyleSheet("resources/svenali-bs.css", "all");
    WApplication *app = WApplication::instance();
    cout << "Approot: " << app->appRoot() << endl;
    messageResourceBundle().use(app->appRoot() + "forms/settings");
    
    //initialiseCss();
    
    root()->addWidget(make_unique<WaveriderGUI>(radio_server));
}

WaveriderApplication::~WaveriderApplication()
{
  cout << "Waverider::Application: cleanup" << endl;
}

void WaveriderApplication::initialiseCss() {
  cout << "Debug: " << "Initialising CSS App" << endl; 

  auto aStyleRule = make_unique<WCssTextRule>("a", "color: #337ab7; text-decoration: none;");
  
  styleSheet().addRule(move(aStyleRule));
}