/* Copyright (C) 2006 Barış Metin <baris@pardus.org.tr>
 * Copyright (C) 2007 Serkan Kaba <serkan_kaba@yahoo.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef ZEMBEREK_H
#define ZEMBEREK_H

#include <dbus/dbus-glib.h>
#include <glib.h>

using namespace std;

class Zemberek
{
public:
    Zemberek();
    ~Zemberek();
    
    int checkWord(const char* word) const;
    char** suggestWord(const char* word, size_t *out_n_suggs);
    //char *error;
private:
    DBusGConnection *connection;
    DBusGProxy *proxy;
};
#endif
