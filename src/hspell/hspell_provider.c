/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2003 Yaacov Zamir
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <hspell.h>

#include "enchant.h"
#include "enchant-provider.h"

ENCHANT_PLUGIN_DECLARE("Hspell")

static int
hspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	return ( hspell_check ( (hspell_session*)(me->user_data) , word, len ) );
}

static char **
hspell_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{	
	char **sugg_arr = NULL;
	size_t n_suggestions=0;

	// suggest only for hebrew words
	// FIXME: what about utf-8 ?
	//        this is just for iso8859-8
	*out_n_suggs = n_suggestions;
	if ( word[0] < 'à' || word[0] > 'ú' ) return NULL;
 
	sugg_arr = hspell_suggest ( (hspell_session*)(me->user_data) , 
				word, len, &n_suggestions);

	*out_n_suggs = n_suggestions;
	return ( sugg_arr );
}

static void
hspell_dict_add_to_personal (EnchantDict * me,
			     const char *const word, size_t len)
{
	hspell_add_to_personal ((hspell_session*)(me->user_data),
			    word, len);
}

static void
hspell_dict_add_to_session (EnchantDict * me,
			    const char *const word, size_t len)
{
	hspell_add_to_session ((hspell_session*)(me->user_data),
			    word, len);
}

static void
hspell_dict_store_replacement (struct str_enchant_dict * me,
			       const char *const mis, size_t mis_len,
			       const char *const cor, size_t cor_len)
{
	hspell_store_replacement ((hspell_session*)(me->user_data),
			    mis, mis_len, cor, cor_len);
}

static void
hspell_dict_free_suggestions (EnchantDict * me, char **str_list)
{
	hspell_free_suggestions ( (hspell_session*)(me->user_data) , str_list);
}

static EnchantDict *
hspell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict;
	hspell_session* my_session;

	// try to set a new session
	my_session = hspell_session_new ();
	
	if ( !my_session)
	{
		enchant_provider_set_error (me, "can't create new session.");
		return NULL;
	}

	dict = g_new0 (EnchantDict, 1);
	dict->user_data = (void *) my_session;
	dict->check = hspell_dict_check;
	dict->suggest = hspell_dict_suggest;
	dict->add_to_personal = hspell_dict_add_to_personal;
	dict->add_to_session = hspell_dict_add_to_session;
	dict->store_replacement = hspell_dict_store_replacement;
	dict->free_suggestions = hspell_dict_free_suggestions;
	
	return dict;
}

static void
hspell_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	if  ( me->user_data )
		hspell_session_unref  ( me->user_data ); 
	g_free (dict);
}

static int
hspell_provider_dictionary_exists (struct str_enchant_provider * me,
				   const char *const tag)
{
	// cheak if tag is he[_IL.something]
	if ( tag[0] == 'h' && tag[1] == 'e' ) 
	{
		return TRUE;
	} else {
		return FALSE;
	}
}

static void
hspell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static char *
hspell_provider_identify (EnchantProvider * me)
{
	return "hspell";
}

static char *
hspell_provider_describe (EnchantProvider * me)
{
	return "Hspell Provider";
}

#ifdef __cplusplus
extern "C" {
#endif

ENCHANT_MODULE_EXPORT (EnchantProvider *) 
init_enchant_provider (void)
{
	EnchantProvider *provider;
	
	provider = g_new0 (EnchantProvider, 1);
	provider->dispose = hspell_provider_dispose;
	provider->request_dict = hspell_provider_request_dict;
	provider->dispose_dict = hspell_provider_dispose_dict;
	provider->dictionary_exists = hspell_provider_dictionary_exists;
	provider->identify = hspell_provider_identify;
	provider->describe = hspell_provider_describe;

	return provider;
}

#ifdef __cplusplus
}
#endif
