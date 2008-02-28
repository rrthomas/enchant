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

#include "zemberek.h"

Zemberek::Zemberek()
{

  GError *Error;
  g_type_init ();

  Error = NULL;
  connection = dbus_g_bus_get (DBUS_BUS_SYSTEM,
                               &Error);
  if (connection == NULL) {
      //g_stpcpy(error,Error->message);
      g_error_free (Error);
  }
  proxy = dbus_g_proxy_new_for_name (connection,
                                     "net.zemberekserver.server.dbus",
                                     "/net/zemberekserver/server/dbus/ZemberekDbus",
                                     "net.zemberekserver.server.dbus.ZemberekDbusInterface");
}


Zemberek::~Zemberek()
{
    if(proxy)
	    g_object_unref (proxy);
}


int Zemberek::checkWord(const char* word ) const
{
    gboolean result;
    GError *Error;
    Error=NULL;
    if (!dbus_g_proxy_call (proxy, "kelimeDenetle", &Error,
    	G_TYPE_STRING,word,G_TYPE_INVALID,
    	G_TYPE_BOOLEAN, &result, G_TYPE_INVALID)) {
    	//g_stpcpy(error,Error->message);
    	g_error_free (Error);
    	return -1;
    }
    else {
    	if (result)
    		return 0;
    	else
    		return 1;
    }
}


char** Zemberek::suggestWord(const char* word, size_t *out_n_suggs)
{
    char** suggs;
    GError *Error;
    Error=NULL;
    if (!dbus_g_proxy_call (proxy, "oner", &Error,
    	G_TYPE_STRING,word,G_TYPE_INVALID,
    	G_TYPE_STRV, &suggs,G_TYPE_INVALID)) {
    	//g_stpcpy(error,Error->message);
    	g_error_free (Error);
    	return NULL;
    }
    *out_n_suggs = g_strv_length(suggs);
    return suggs;
}
