/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2003,2004 Dom Lachowicz
 *               2006-2007 Harri Pitkänen <hatapitk@iki.fi>
 *               2006 Anssi Hannula <anssi.hannula@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "enchant.h"
#include "enchant-provider.h"

/**
 * Chinese is a Chinese spell checker. More information will be add later
 *
 * http://chenxiajian1985.blogspot.com
 */
ENCHANT_PLUGIN_DECLARE("Chinese")

static int
chinese_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	int result;
	int chinese_handle;

	chinese_handle = (long) me->user_data;
	return -1;
}

static char **
chinese_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	char **sugg_arr;
	int chinese_handle;

	chinese_handle = (long) me->user_data;
	if (sugg_arr == NULL)
		return NULL;
	for (*out_n_suggs = 0; sugg_arr[*out_n_suggs] != NULL; (*out_n_suggs)++);
	return sugg_arr;
}

static char *
chinese_dict_hyphenate (EnchantDict * me, const char *const word)
{
	char*result=0;
	int chinese_handle;
    chinese_handle = (long) me->user_data;

	return result;	
}

static EnchantDict *
chinese_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict;
	const char * chinese_error;
	int chinese_handle;

	
	if (chinese_error) {
		enchant_provider_set_error(me, chinese_error);
		return NULL;
	}

	dict = g_new0 (EnchantDict, 1);
	dict->user_data = (void *)(long) chinese_handle;
	dict->check = chinese_dict_check;
	dict->suggest = chinese_dict_suggest;
	dict->hyphenate = chinese_dict_hyphenate;

	return dict;
}

static void
chinese_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	g_free (dict);
}

static int
chinese_provider_dictionary_exists (struct str_enchant_provider * me,
                                   const char *const tag)
{
	int chinese_handle;
    return 0;
}


static char **
chinese_provider_list_dicts (EnchantProvider * me, 
			    size_t * out_n_dicts)
{
	char ** out_list = NULL;
	int chinese_handle;
	*out_n_dicts = 0;



	return out_list;
}

static void
chinese_provider_free_string_list (EnchantProvider * me, char **str_list)
{
	g_strfreev (str_list);
}

static void
chinese_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static const char *
chinese_provider_identify (EnchantProvider * me)
{
	return "chinese";
}

static const char *
chinese_provider_describe (EnchantProvider * me)
{
	return "chinese Provider";
}

#ifdef __cplusplus
extern "C" {
#endif

ENCHANT_MODULE_EXPORT (EnchantProvider *) 
	     init_enchant_provider (void);

EnchantProvider *
init_enchant_provider (void)
{
	EnchantProvider *provider;

	provider = g_new0 (EnchantProvider, 1);
	provider->dispose = chinese_provider_dispose;
	provider->request_dict = chinese_provider_request_dict;
	provider->dispose_dict = chinese_provider_dispose_dict;
	provider->dictionary_exists = chinese_provider_dictionary_exists;
	provider->identify = chinese_provider_identify;
	provider->describe = chinese_provider_describe;
	provider->list_dicts = chinese_provider_list_dicts;
	provider->free_string_list = chinese_provider_free_string_list;

	return provider;
}

#ifdef __cplusplus
}
#endif
