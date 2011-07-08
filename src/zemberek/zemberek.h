/* Copyright (C) 2006 Barış Metin <baris@pardus.org.tr>
 * Copyright (C) 2007 Serkan Kaba <serkan_kaba@yahoo.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02110-1301, USA.
 *
 * In addition, as a special exception, Dom Lachowicz
 * gives permission to link the code of this program with
 * non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#ifndef ZEMBEREK_H
#define ZEMBEREK_H

#include <dbus/dbus-glib.h>
#include <glib.h>

class Zemberek
{
public:
    Zemberek();
    ~Zemberek();
    
    int checkWord(const char* word) const;
    char** suggestWord(const char* word, size_t *out_n_suggs);
<<<<<<< .mine
<<<<<<< .mine
	char** hyphenate(const char* word, size_t *out_n_suggs);
=======
	char* hyphenate(const char* word);
>>>>>>> .theirs
=======
	char* hyphenate(const char* word);




>>>>>>> .theirs

private:
    DBusGConnection *connection;
    DBusGProxy *proxy;
};

bool zemberek_service_is_running ();
#endif
