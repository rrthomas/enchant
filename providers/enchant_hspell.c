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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

/*
 * Hspell was written by Nadav Har'El and Dan Kenigsberg.
 * See: http://www.ivrix.org.il/projects/spell-checker/
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <hspell.h>

#include "enchant-provider.h"

/**
 * convert struct corlist to **char
 * the **char must be g_freed
 */
static gchar **
corlist2strv (struct corlist *cl, size_t nb_sugg)
{
	char **sugg_arr = NULL;
	if (nb_sugg > 0)
		{
			sugg_arr = g_new0 (char *, nb_sugg + 1);
			for (size_t i = 0; i < nb_sugg; i++)
				{
					gsize len;
					const char *sugg = corlist_str (cl, i);
					if (sugg)
						sugg_arr[i] = g_convert (sugg,
									 strlen (sugg),
									 "utf-8", "iso8859-8", NULL, &len, NULL);
				}
		}

	return sugg_arr;
}

static char *
hspell_convert_to_iso8859_8 (EnchantDict *me, const char *const word, size_t len)
{
	/* convert to iso 8859-8 */
	gsize length;
	char *iso_word = g_convert (word, len, "iso8859-8", "utf-8", NULL, &length, NULL);
	if (iso_word == NULL)
		enchant_dict_set_error (me, "word not valid Hebrew (could not be converted to ISO-8859-8)");
	return iso_word;
}

static int
hspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	struct dict_radix *hspell_dict = (struct dict_radix *)me->user_data;
	char *iso_word = hspell_convert_to_iso8859_8 (me, word, len);
	g_return_val_if_fail (iso_word, -1);

	/* check */
	int preflen;
	int res = hspell_check_word (hspell_dict, iso_word, &preflen);

	/* if not correct try gimatria */
	if (res != 1)
		res = hspell_is_canonic_gimatria (iso_word) != 0;

	g_free (iso_word);

	return (res != 1);
}

static char **
hspell_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	struct dict_radix *hspell_dict = (struct dict_radix *)me->user_data;
	char *iso_word = hspell_convert_to_iso8859_8 (me, word, len);
	g_return_val_if_fail (iso_word, NULL);

	/* get suggestions */
	struct corlist cl;
	corlist_init (&cl);
	hspell_trycorrect (hspell_dict, iso_word, &cl);

	*out_n_suggs = corlist_n (&cl);
	char **sugg_arr = corlist2strv (&cl, *out_n_suggs);
	corlist_free (&cl);
	g_free (iso_word);

	return sugg_arr;
}

static EnchantDict *
hspell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	if(!((strlen(tag) >= 2) && tag[0] == 'h' && tag[1] == 'e'))
		return NULL;

	/* try to set a new session */
	struct dict_radix *hspell_dict = NULL;
	int dict_flag = hspell_init (&hspell_dict, HSPELL_OPT_DEFAULT);

	if (dict_flag != 0 || !hspell_dict)
		{
			enchant_provider_set_error (me, "can't create new dict.");
			return NULL;
		}

	EnchantDict *dict = g_new0 (EnchantDict, 1);
	dict->user_data = (void *) hspell_dict;
	dict->check = hspell_dict_check;
	dict->suggest = hspell_dict_suggest;

	return dict;
}

static void
hspell_provider_dispose_dict (EnchantProvider * me G_GNUC_UNUSED, EnchantDict * dict)
{
	struct dict_radix *hspell_dict = (struct dict_radix *)dict->user_data;
	hspell_uninit (hspell_dict);
	g_free (dict);
}

/* test for the existence of, then return $prefix/share/hspell/hebrew.wgz */

static char **
hspell_provider_list_dicts (EnchantProvider * me G_GNUC_UNUSED,
			    size_t * out_n_dicts)
{
	const char * dictionary_path = hspell_get_dictionary_path();
	char ** out_list = NULL;
	*out_n_dicts = 0;

	if(dictionary_path && *dictionary_path && g_file_test (dictionary_path, G_FILE_TEST_EXISTS)) {
		out_list = g_new0 (char *, 2);
		out_list[(*out_n_dicts)++] = g_strdup ("he");
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
hspell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static const char *
hspell_provider_identify (EnchantProvider * me G_GNUC_UNUSED)
{
	return "hspell";
}

static const char *
hspell_provider_describe (EnchantProvider * me G_GNUC_UNUSED)
{
	return "Hspell Provider";
}

EnchantProvider *init_enchant_provider (void);

EnchantProvider *
init_enchant_provider (void)
{
	EnchantProvider *provider = g_new0 (EnchantProvider, 1);
	provider->dispose = hspell_provider_dispose;
	provider->request_dict = hspell_provider_request_dict;
	provider->dispose_dict = hspell_provider_dispose_dict;
	provider->dictionary_exists = hspell_provider_dictionary_exists;
	provider->identify = hspell_provider_identify;
	provider->describe = hspell_provider_describe;
	provider->list_dicts = hspell_provider_list_dicts;

	return provider;
}
