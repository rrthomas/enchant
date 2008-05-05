/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2003 Yaacov Zamir
 * Copyright (C) 2004 Dom Lachowicz
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
 * In addition, as a special exception, Dom Lachowicz and Yaacov Zamir
 * give permission to link the code of this program with
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

ENCHANT_PLUGIN_DECLARE ("Hspell")

/**
 * hspell helper functions
 */
     
/**
 * is hebrew
 * return TRUE if iso hebrew ( must be null terminated )
 */
static int is_hebrew (const char *const iso_word, gsize len)
{
	int i;
	
	for ( i = 0; (i < len) && (iso_word[i]); i++ )
		{
			/* if not a hebrew alphabet or " ` ' using iso-8859-8 encoding */
			if ( (iso_word[i] < (char)224 || iso_word[i] > (char)250 ) && /* alef to tav */
			     (iso_word[i] < (char)146 || iso_word[i] > (char)148 ) && /* ` etc... */
			     ( iso_word[i] !=(char)34 ) && /* " */
			     ( iso_word[i] !=(char)39 ) ) /* ' */
				return FALSE;
		}
	
	return TRUE;
}

/**
 * convert struct corlist to **char
 * the **char must be g_freed
 */
static gchar **
corlist2strv (struct corlist *cl, size_t nb_sugg)
{
	int i;
	gsize len;
	char **sugg_arr = NULL;
	const char *sugg;
	
	if (nb_sugg > 0)
		{
			sugg_arr = g_new0 (char *, nb_sugg + 1);
			for (i = 0; i < nb_sugg; i++)
				{
					sugg = corlist_str (cl, i);
					if (sugg)
						sugg_arr[i] = g_convert (sugg,
									 strlen (sugg),
									 "utf-8", "iso8859-8", NULL, &len, NULL);
				}
		}
	
	return sugg_arr;
}

/**
 * end of helper functions
 */

static int
hspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	int res;
	char *iso_word;
	gsize length;
	int preflen;
	struct dict_radix *hspell_dict;
	
	hspell_dict = (struct dict_radix *)me->user_data;
	
	/* convert to iso 8859-8 */
	iso_word = g_convert (word, len, "iso8859-8", "utf-8", NULL, &length, NULL);
	
	/* check if hebrew ( if not hebrew give it the benefit of a doubt ) */
	if (iso_word == NULL || !is_hebrew (iso_word, length))
		{
			if (iso_word)
				g_free (iso_word);
			return FALSE;
		}
	
	/* check */
	res = hspell_check_word (hspell_dict, iso_word, &preflen);
	
	/* if not correct try gimatria */
	if (res != 1)
		{
			res = hspell_is_canonic_gimatria (iso_word);
			if (res != 0)
				res = 1;
		}
	
	/* free the word */
	g_free (iso_word);
	
	return (res != 1);
}

static char **
hspell_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	
	int res;
	gsize length;
	char *iso_word;
	char **sugg_arr = NULL;
	struct corlist cl;
	struct dict_radix *hspell_dict;
	
	hspell_dict = (struct dict_radix *)me->user_data;
	
	/* convert to iso 8859-8 */
	iso_word = g_convert (word, len, "iso8859-8", "utf-8", NULL, &length, NULL);
	
	/* check if hebrew ( if not hebrew cant do anything ) */
	if (iso_word == NULL || !is_hebrew (iso_word, length))
		{
			if (iso_word != NULL)
				g_free (iso_word);
			return NULL;
		}

	/* get suggestions */
	corlist_init (&cl);
	hspell_trycorrect (hspell_dict, iso_word, &cl);
	
	/* set size of list */
	*out_n_suggs = corlist_n (&cl);
	
	/* convert suggestion list to strv list */
	sugg_arr = corlist2strv (&cl, *out_n_suggs);
	
	/* free the list */
	corlist_free (&cl);
	
	/* free the word */
	g_free (iso_word);
	
	return sugg_arr;	
}

static EnchantDict *
hspell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict;
	int dict_flag = 0;
	struct dict_radix *hspell_dict = NULL;
	
	if(!((strlen(tag) >= 2) && tag[0] == 'h' && tag[1] == 'e'))
		return NULL;
	
	/* try to set a new session */
	dict_flag = hspell_init (&hspell_dict, HSPELL_OPT_DEFAULT);
	
	if (dict_flag != 0 || !hspell_dict)
		{
			enchant_provider_set_error (me, "can't create new dict.");
			return NULL;
		}
	
	dict = g_new0 (EnchantDict, 1);
	dict->user_data = (void *) hspell_dict;
	dict->check = hspell_dict_check;
	dict->suggest = hspell_dict_suggest;
	
	return dict;
}

static void
hspell_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	struct dict_radix *hspell_dict;
	
	hspell_dict = (struct dict_radix *)dict->user_data;
	
	/* deleting the dict is not posible on hspell ver. < v.0.8 */
#if (HSPELL_VERSION_MAJOR > 0) || (HSPELL_VERSION_MINOR >= 8)
	hspell_uninit (hspell_dict);
#endif
	g_free (dict);
}

/* test for the existence of, then return $prefix/share/hspell/hebrew.wgz */

static char ** 
hspell_provider_list_dicts (EnchantProvider * me, 
			    size_t * out_n_dicts)
{
	const char * dictionary_path;
	char ** out_list = NULL;
	*out_n_dicts = 0;
	
	dictionary_path = hspell_get_dictionary_path();
	
	if(dictionary_path && *dictionary_path && g_file_test (dictionary_path, G_FILE_TEST_EXISTS)) {
		*out_n_dicts = 1;
		
		out_list = g_new0 (char *, 2);
		
		out_list[0] = g_strdup ("he");
	}		
		
	return out_list;
}

static int
hspell_provider_dictionary_exists (struct str_enchant_provider * me,
				   const char *const tag)
{
	(void)me;
	return (!strcmp ("he", tag) || !strcmp ("he_IL", tag));
}

static void
hspell_provider_free_string_list (EnchantProvider * me, char **str_list)
{
	g_strfreev (str_list);
}

static void
hspell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static const char *
hspell_provider_identify (EnchantProvider * me)
{
	return "hspell";
}

static const char *
hspell_provider_describe (EnchantProvider * me)
{
	return "Hspell Provider";
}

#ifdef __cplusplus
extern "C"
{
#endif
	
ENCHANT_MODULE_EXPORT (EnchantProvider *) 
	     init_enchant_provider (void);

EnchantProvider *
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
	provider->list_dicts = hspell_provider_list_dicts;
	provider->free_string_list = hspell_provider_free_string_list;

	return provider;
}

#ifdef __cplusplus
}
#endif
