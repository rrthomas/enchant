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
 * In addition, as a special exception, Dom Lachowicz and Yaacov Zamir
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

ENCHANT_PLUGIN_DECLARE ("Hspell")

/**
 * hspell helper functions
 */
     
/**
 * is hebrew
 * return TRUE if iso hebrew ( must be null terminated )
 */
static int is_hebrew (const char *const iso_word)
{
	int i = 0;
	
	while ( iso_word[i] )
		{
			/* if not a hebrew alphabet or " ` ' */
			if ( (iso_word[i] < 'à' || iso_word[i] > 'ú') && /* alef to tav */
			     (iso_word[i] < (char)146 || iso_word[i] > (char)148 ) && /* ` etc... */
			     ( iso_word[i] !=(char)34 ) && /* " */
			     ( iso_word[i] !=(char)39 ) ) /* ' */
				return FALSE;
			i++;
		}
	
	return TRUE;
}

/**
 * convert struct corlist to **char
 * the **char must be g_freed
 */
static gchar **
corlist2strv (struct corlist *cl)
{
	int i;
	gsize len;
	char **sugg_arr = NULL;
	const char *sugg;
	
	if (corlist_n (cl) > 0)
		{
			sugg_arr = g_new0 (char *, corlist_n (cl) + 1);
			for (i = 0; i < corlist_n (cl); i++)
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

/**
 * this is global !??
 */
static struct dict_radix *hspell_common_dict = NULL;
static size_t dict_ref_cnt = 0;

static int
hspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	int res;
	char *iso_word;
	gsize length;
	int preflen;

	/* convert to iso 8859-8 */
	iso_word = g_convert (word, len, "iso8859-8", "utf-8", NULL, &length, NULL);
	
	/* check if hebrew ( if not hebrew give it the benefit of a doubt ) */
	if (iso_word == NULL || !is_hebrew (iso_word))
		{
			if (iso_word)
				g_free (iso_word);
			return TRUE;
		}

	/* check */
	res = hspell_check_word (hspell_common_dict, iso_word, &preflen);
	
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
	
	/* convert to iso 8859-8 */
	iso_word = g_convert (word, len, "iso8859-8", "utf-8", NULL, &length, NULL);
	
	/* check if hebrew ( if not hebrew cant do anything ) */
	if (iso_word == NULL || !is_hebrew (iso_word))
		{
			if (iso_word != NULL)
				g_free (iso_word);
			return NULL;
		}

	/* get suggestions */
	corlist_init (&cl);
	hspell_trycorrect (hspell_common_dict, iso_word, &cl);
	
	/* set size of list */
	*out_n_suggs = corlist_n (&cl);
	
	/* convert suggestion list to strv list */
	sugg_arr = corlist2strv (&cl);
	
	/* free the list */
	corlist_free (&cl);
	
	/* free the word */
	g_free (iso_word);
	
	return sugg_arr;	
}

static void
hspell_dict_free_suggestions (EnchantDict * me, char **str_list)
{
	g_strfreev (str_list);
}

static int
hspell_provider_dictionary_exists (struct str_enchant_provider *me,
				   const char *const tag)
{
	/* cheak if tag is he[_IL.something] */
	if ((strlen(tag) >= 2) && tag[0] == 'h' && tag[1] == 'e')
		{
			return TRUE;
		}
	else
		{
			return FALSE;
		}
}

static EnchantDict *
hspell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict;
	int dict_flag = 0;

	if(!hspell_provider_dictionary_exists(me, tag))
		return NULL;
	
	/* try to set a new session */
	if (hspell_common_dict == NULL)
		{
			dict_flag = hspell_init (&hspell_common_dict, HSPELL_OPT_DEFAULT);
		}
	
	if (dict_flag != 0)
		{
			enchant_provider_set_error (me, "can't create new dict.");
			return NULL;
		}
	else 
		{
			dict_ref_cnt++;
		}
	
	dict = g_new0 (EnchantDict, 1);
	dict->check = hspell_dict_check;
	dict->suggest = hspell_dict_suggest;
	dict->free_suggestions = hspell_dict_free_suggestions;
	
	return dict;
}

static void
hspell_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	dict_ref_cnt--;
	if (dict_ref_cnt == 0) 
		{
			/* deleting the dict is not posible yet (hspell v.0.7) :-( */
		}
	g_free (dict);
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
extern "C"
{
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
