/* enchant
 * Copyright (C) 2003 Yaacov Zamir
 * Copyright (C) 2004 Dom Lachowicz
 * Copyright (C) 2017-2025 Reuben Thomas
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders
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

#define HSPELL_MAIN_SUFFIX ".wgz"

/**
 * convert struct corlist to **char
 * the **char must be g_freed
 */
static gchar **
corlist2strv (struct corlist *cl, size_t nb_sugg)
{
	char **sugg_arr = g_new0 (char *, nb_sugg + 1);
	if (sugg_arr)
		for (size_t i = 0; i < nb_sugg; i++) {
			gsize len;
			const char *sugg = corlist_str (cl, i);
			sugg_arr[i] = g_convert (sugg,
						 strlen (sugg),
						 "utf-8", "iso8859-8", NULL, &len, NULL);
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
	if (iso_word == NULL) {
		g_warning ("%s: Can't convert word to iso8859-8", __FUNCTION__);
		return -1;
	}

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
	if (iso_word == NULL) {
		g_warning ("%s: Can't convert word to iso8859-8", __FUNCTION__);
		return NULL;
	}

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

static GSList *
hspell_provider_enum_dict_files (const char * const directory)
{
	GSList * out_dicts = NULL;
	GDir * dir = g_dir_open (directory, 0, NULL);
	if (dir) {
		const char * entry;
		while ((entry = g_dir_read_name (dir)) != NULL) {
			char * utf8_entry = g_filename_to_utf8 (entry, -1, NULL, NULL, NULL);
			if (utf8_entry && g_strrstr (utf8_entry, HSPELL_MAIN_SUFFIX) != NULL) {
				char * dic = g_build_filename(directory, utf8_entry, NULL);
				char * desc_file = g_strconcat(dic, ".desc", NULL);
				if (desc_file && g_file_test(desc_file, G_FILE_TEST_EXISTS) != 0)
					out_dicts = g_slist_append (out_dicts, dic);
				else
					g_free (dic);
				g_free (desc_file);
			}
			g_free (utf8_entry);
		}

		g_dir_close (dir);
	}
	return out_dicts;
}

static EnchantDict *
hspell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	g_debug("hspell_provider_request_dict");
	gboolean dict_found = FALSE;
	char * user_dict_dir = enchant_provider_get_user_dict_dir (me);
	GSList * dict_files = hspell_provider_enum_dict_files (user_dict_dir);
	for (GSList * dict_file = dict_files;
	     dict_file != NULL;
	     dict_file = dict_file->next) {
		gchar * dict_filename = (gchar *)dict_file->data;
		gchar * basename = g_path_get_basename (dict_filename);
		gchar * tag_end = g_strrstr (basename, HSPELL_MAIN_SUFFIX);
		if (tag_end != NULL) {
			*tag_end = '\0';
			if (!strcmp (basename, tag)) {
				hspell_set_dictionary_path (dict_filename);
				g_free (basename);
				dict_found = TRUE;
				break;
			}
		}
		g_free (basename);
	}
	g_free (user_dict_dir);
	g_slist_free (dict_files);

	if (dict_found == FALSE && strcmp (tag, "he") && strcmp (tag, "he_IL"))
		return NULL;

	struct dict_radix *hspell_dict = NULL;
	int dict_flag = hspell_init (&hspell_dict, HSPELL_OPT_DEFAULT);

	if (dict_flag != 0 || !hspell_dict) {
		enchant_provider_set_error (me, "cannot get requested dictionary");
		return NULL;
	}

	EnchantDict *dict = enchant_dict_new ();
	if (dict == NULL)
		return NULL;
	dict->user_data = (void *) hspell_dict;
	dict->check = hspell_dict_check;
	dict->suggest = hspell_dict_suggest;

	return dict;
}

static void
hspell_provider_dispose_dict (EnchantProvider * me _GL_UNUSED, EnchantDict * dict)
{
	struct dict_radix *hspell_dict = (struct dict_radix *)dict->user_data;
	hspell_uninit (hspell_dict);
}

/* Find any user dictionaries, and test default dictionary. */

static char **
hspell_provider_list_dicts (EnchantProvider * me, size_t * out_n_dicts)
{
	*out_n_dicts = 0;

	char * user_dict_dir = enchant_provider_get_user_dict_dir (me);
	GSList * dict_files = hspell_provider_enum_dict_files (user_dict_dir);
	guint n_user_dicts = g_slist_length (dict_files);
	char ** out_list = g_new0 (char *, n_user_dicts + 3);
	if (out_list)
		for (guint i = 0; i < n_user_dicts; i++) {
			gchar * dict_file = g_slist_nth_data (dict_files, i);
			gchar * basename = g_path_get_basename (dict_file);
			gchar * tag_end = g_strrstr (basename, HSPELL_MAIN_SUFFIX);
			if (tag_end != NULL) {
				*tag_end = '\0';
				out_list[(*out_n_dicts)++] = basename;
			} else
				g_free (basename);
		}
	g_free (user_dict_dir);
	g_slist_free_full (dict_files, g_free);

	const char * dictionary_path = hspell_get_dictionary_path();
	if(out_list && dictionary_path && *dictionary_path && g_file_test (dictionary_path, G_FILE_TEST_EXISTS)) {
		out_list[(*out_n_dicts)++] = g_strdup ("he");
		out_list[(*out_n_dicts)++] = g_strdup ("he_IL");
	}

	return out_list;
}

static const char *
hspell_provider_identify (EnchantProvider * me _GL_UNUSED)
{
	return "hspell";
}

static const char *
hspell_provider_describe (EnchantProvider * me _GL_UNUSED)
{
	return "Hspell Provider";
}

EnchantProvider *init_enchant_provider (void);

EnchantProvider *
init_enchant_provider (void)
{
	EnchantProvider *provider = enchant_provider_new ();
	provider->request_dict = hspell_provider_request_dict;
	provider->dispose_dict = hspell_provider_dispose_dict;
	provider->identify = hspell_provider_identify;
	provider->describe = hspell_provider_describe;
	provider->list_dicts = hspell_provider_list_dicts;

	return provider;
}
