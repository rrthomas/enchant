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

#include <glib.h>

#include "enchant.h"
#include "enchant-provider.h"

#include "zemberek.h"

ENCHANT_PLUGIN_DECLARE("Zemberek")


static int
zemberek_dict_check (EnchantDict * me, const char *const word, size_t len)
{
    Zemberek *checker;
    checker = (Zemberek *) me->user_data;
    int result = checker->checkWord(word);
    /*
    if(result == -1) {
    	enchant_dict_set_error(me,checker->error);
    	g_free(checker->error);
    }
    */
    return result;
}

static char**
zemberek_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
    Zemberek *checker;
    checker = (Zemberek *) me->user_data;
    char **result = checker->suggestWord (word, out_n_suggs);
    /*
    if(!result) {
    	enchant_dict_set_error(me,checker->error);
    	g_free(checker->error);
    }
    */
    return result;
}

static void
zemberek_provider_dispose(EnchantProvider *me)
{
    g_free(me);
}

static EnchantDict*
zemberek_provider_request_dict(EnchantProvider *me, const char *tag)
{
    Zemberek* checker = new Zemberek();

    if (!checker)
	return NULL;

    EnchantDict* dict = g_new0(EnchantDict, 1);
    dict->user_data = (void *) checker;
    dict->check = zemberek_dict_check;
    dict->suggest = zemberek_dict_suggest;
	
    return dict;
}

static void
zemberek_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
    Zemberek *checker;
    checker = (Zemberek *) dict->user_data;
    delete checker;
    g_free (dict);
}

static const char *
zemberek_provider_identify (EnchantProvider * me)
{
	return "zemberek";
}

static const char *
zemberek_provider_describe (EnchantProvider * me)
{
	return "Zemberek Provider";
}

static void
zemberek_provider_free_string_list (EnchantProvider * me, char **str_list)
{
	g_strfreev (str_list);
}

static char ** 
zemberek_provider_list_dicts (EnchantProvider * me, 
			      size_t * out_n_dicts)
{
	char ** out_list = NULL;
	*out_n_dicts = 1;
	out_list = g_new0 (char *, 2);
	out_list[0] = g_strdup ("tr");
		
	return out_list;
}

extern "C" {

ENCHANT_MODULE_EXPORT(EnchantProvider *) 
  init_enchant_provider(void);

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
