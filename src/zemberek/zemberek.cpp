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
 * Boston, MA 02111-1307, USA.
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

#include "zemberek.h"

bool zemberek_service_is_running ()
{
  DBusGConnection *connection;
  DBusGProxy *proxy;

  GError *Error = NULL;
  g_type_init ();

  connection = dbus_g_bus_get (DBUS_BUS_SYSTEM,
                               &Error);
  if (connection == NULL) {
      g_error_free (Error);
      return false;
  }
  proxy = dbus_g_proxy_new_for_name (connection,
                                     "net.zemberekserver.server.dbus",
                                     "/net/zemberekserver/server/dbus/ZemberekDbus",
                                     "net.zemberekserver.server.dbus.ZemberekDbusInterface");

  dbus_g_connection_unref (connection);
  if (proxy == NULL) {
    return false;
  }

   g_object_unref (proxy);
   return true;
}

Zemberek::Zemberek()
  : connection(NULL), proxy(NULL)
{
  GError *Error = NULL;
  g_type_init ();

  connection = dbus_g_bus_get (DBUS_BUS_SYSTEM,
                               &Error);
  if (connection == NULL) {
      g_error_free (Error);
      throw "couldn't connect to the system bus";
  }
  proxy = dbus_g_proxy_new_for_name (connection,
                                     "net.zemberekserver.server.dbus",
                                     "/net/zemberekserver/server/dbus/ZemberekDbus",
                                     "net.zemberekserver.server.dbus.ZemberekDbusInterface");

  if (proxy == NULL) {
    throw "couldn't connect to the Zemberek service";
  }
}


Zemberek::~Zemberek()
{
    if(proxy)
	    g_object_unref (proxy);
    if(connection)
	    dbus_g_connection_unref (connection);
}


int Zemberek::checkWord(const char* word) const
{
    gboolean result;
    GError *Error = NULL;
    if (!dbus_g_proxy_call (proxy, "kelimeDenetle", &Error,
    	G_TYPE_STRING,word,G_TYPE_INVALID,
    	G_TYPE_BOOLEAN, &result, G_TYPE_INVALID)) {
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
    GError *Error = NULL;
    if (!dbus_g_proxy_call (proxy, "oner", &Error,
    	G_TYPE_STRING,word,G_TYPE_INVALID,
    	G_TYPE_STRV, &suggs,G_TYPE_INVALID)) {
    	g_error_free (Error);
    	return NULL;
    }
    *out_n_suggs = g_strv_length(suggs);
    return suggs;
}
