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

#include "config.h"

#include <glib.h>
#include <string.h>

#include "enchant.h"
#include "enchant-provider.h"
#include "unused-parameter.h"

#include "zemberek.h"


extern "C" {

ENCHANT_MODULE_EXPORT(EnchantProvider *)
  init_enchant_provider(void);

static int
zemberek_dict_check (EnchantDict * me, const char *const word, size_t len _GL_UNUSED_PARAMETER)
{
    Zemberek *checker;
    checker = (Zemberek *) me->user_data;
    return checker->checkWord(word);
}

static char**
zemberek_dict_suggest (EnchantDict * me, const char *const word,
                       size_t len _GL_UNUSED_PARAMETER, size_t * out_n_suggs)
{
    Zemberek *checker;
    checker = (Zemberek *) me->user_data;
    return checker->suggestWord (word, out_n_suggs);
}

static void
zemberek_provider_dispose(EnchantProvider *me)
{
    g_free(me);
}

static EnchantDict*
zemberek_provider_request_dict(EnchantProvider *me _GL_UNUSED_PARAMETER, const char *tag)
{
    if (!((strcmp(tag, "tr") == 0) || (strncmp(tag, "tr_", 3) == 0)))
	return NULL; // only handle turkish

    try
      {
	Zemberek* checker = new Zemberek();

	EnchantDict* dict = g_new0(EnchantDict, 1);
	dict->user_data = (void *) checker;
	dict->check = zemberek_dict_check;
	dict->suggest = zemberek_dict_suggest;

	return dict;
      }
    catch(...)
      {
	// will fail if zemberek service isn't running
	return NULL;
      }
}

static void
zemberek_provider_dispose_dict (EnchantProvider * me _GL_UNUSED_PARAMETER, EnchantDict * dict)
{
    Zemberek *checker;
    checker = (Zemberek *) dict->user_data;
    delete checker;
    g_free (dict);
}

static const char *
zemberek_provider_identify (EnchantProvider * me _GL_UNUSED_PARAMETER)
{
	return "zemberek";
}

static const char *
zemberek_provider_describe (EnchantProvider * me _GL_UNUSED_PARAMETER)
{
	return "Zemberek Provider";
}

static void
zemberek_provider_free_string_list (EnchantProvider * me _GL_UNUSED_PARAMETER, char **str_list)
{
	g_strfreev (str_list);
}

static char **
zemberek_provider_list_dicts (EnchantProvider * me _GL_UNUSED_PARAMETER,
			      size_t * out_n_dicts)
{
  if (!zemberek_service_is_running ())
    {
	*out_n_dicts = 0;
	return NULL;
    }
  else
    {
	char ** out_list = NULL;
	*out_n_dicts = 1;
	out_list = g_new0 (char *, 2);
	out_list[0] = g_strdup ("tr");

	return out_list;
    }
}

EnchantProvider *
init_enchant_provider(void)
{
    EnchantProvider *provider;

    provider = g_new0(EnchantProvider, 1);
    provider->dispose = zemberek_provider_dispose;
    provider->request_dict = zemberek_provider_request_dict;
    provider->dispose_dict = zemberek_provider_dispose_dict;
    provider->identify = zemberek_provider_identify;
    provider->describe = zemberek_provider_describe;
    provider->list_dicts = zemberek_provider_list_dicts;
    provider->free_string_list = zemberek_provider_free_string_list;

    return provider;
}

}
