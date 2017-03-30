/* enchant
 * Copyright (C) 2003, 2004 Dom Lachowicz
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <glib.h>
#include <gmodule.h>
#include <glib/gstdio.h>
#include <locale.h>

#include "enchant.h"
#include "unused-parameter.h"
#include "relocatable.h"
#include "configmake.h"

/********************************************************************************/

struct str_enchant_broker
{
	GHashTable *dict_map;		/* map of language tag -> dictionary */
	gchar * error;
};

typedef struct str_enchant_session
{
	GHashTable *session_include;
	GHashTable *session_exclude;

	char * personal_filename;
	char * exclude_filename;
	char * language_tag;

	char * error;
} EnchantSession;

typedef struct str_enchant_dict_private_data
{
	unsigned int reference_count;
	EnchantSession* session;
} EnchantDictPrivateData;

/********************************************************************************/
/********************************************************************************/

/* Relocate a path and ensure the result is allocated on the heap */
static char *
enchant_relocate (const char *path)
{
	char *newpath = (char *) relocate (path);
	if (path == newpath)
		newpath = strdup (newpath);
	return newpath;
}

static void
enchant_ensure_dir_exists (const char* dir)
{
	if (dir && !g_file_test (dir, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
		{
			(void)g_remove (dir);
			g_mkdir_with_parents (dir, 0700);
		}
}

ENCHANT_MODULE_EXPORT (char *)
enchant_get_user_config_dir (void)
{
	const gchar * env = g_getenv("ENCHANT_CONFIG_DIR");
	if (env) {
		return g_filename_to_utf8(env, -1, NULL, NULL, NULL);
	}
	return g_build_filename (g_get_user_config_dir (), "enchant", NULL);
}

static GSList *
enchant_get_conf_dirs (void)
{
	GSList *conf_dirs = NULL;

	conf_dirs = g_slist_append (conf_dirs, enchant_relocate (PKGDATADIR));

	char *sysconfdir = enchant_relocate (SYSCONFDIR);
	char *pkgconfdir = g_build_filename (sysconfdir, "enchant", NULL);
	conf_dirs = g_slist_append (conf_dirs, pkgconfdir);
	free (sysconfdir);

	conf_dirs = g_slist_append (conf_dirs, enchant_get_user_config_dir ());

	return conf_dirs;
}

/********************************************************************************/
/********************************************************************************/

static gchar*
enchant_modify_string_chars (gchar *str,
							 gssize len,
							 gchar (*function)(gchar))
{
	gchar* it, *end;

	g_return_val_if_fail (str != NULL, NULL);

	if (len < 0)
		len = strlen (str);

	end = str + len;

	for (it = str; it != end; ++it)
		*it = function (*it);

	return str;
}

static gchar*
enchant_ascii_strup (gchar *str,
					 gssize len)
{
	return enchant_modify_string_chars(str, len, g_ascii_toupper);
}

static gchar*
enchant_ascii_strdown (gchar *str,
						  gssize len)
{
	return enchant_modify_string_chars(str, len, g_ascii_tolower);
}

/* returns TRUE if tag is valid
 * for requires alphanumeric ASCII or underscore
 */
static _GL_ATTRIBUTE_PURE int
enchant_is_valid_dictionary_tag(const char * const tag)
{
	const char * it;
	for (it = tag; *it; ++it)
		{
			if(!g_ascii_isalnum(*it) && *it != '_')
				return 0;
		}

	return it != tag; /*empty tag invalid*/
}

static char *
enchant_normalize_dictionary_tag (const char * const dict_tag)
{
	char * new_tag = strdup (dict_tag);
	char * needle;

	new_tag = g_strstrip (new_tag);

	/* strip off en_GB@euro */
	if ((needle = strchr (new_tag, '@')) != NULL)
		*needle = '\0';

	/* strip off en_GB.UTF-8 */
	if ((needle = strchr (new_tag, '.')) != NULL)
		*needle = '\0';

	/* turn en-GB into en_GB */
	if ((needle = strchr (new_tag, '-')) != NULL)
		*needle = '_';

	/* everything before first '_' is converted to lower case */
	if ((needle = strchr (new_tag, '_')) != NULL) {
			enchant_ascii_strdown(new_tag, needle - new_tag);
			++needle;
			/* everything after first '_' is converted to upper case */
			enchant_ascii_strup(needle, -1);
		}
	else {
			enchant_ascii_strdown(new_tag, -1);
		}

	return new_tag;
}

static char *
enchant_iso_639_from_tag (const char * const dict_tag)
{
	char * new_tag = strdup (dict_tag);
	char * needle;

	if ((needle = strchr (new_tag, '_')) != NULL)
		*needle = '\0';

	return new_tag;
}

static void
enchant_session_destroy (EnchantSession * session)
{
	g_hash_table_destroy (session->session_include);
	g_hash_table_destroy (session->session_exclude);
	g_free (session->personal_filename);
	g_free (session->exclude_filename);
	free (session->language_tag);

	if (session->error)
		g_free (session->error);

	g_free (session);
}

static void
enchant_session_add (EnchantSession * session, const char * const word, size_t len)
{
	char* key = g_strndup (word, len);
	g_hash_table_remove (session->session_exclude, key);
	g_hash_table_insert (session->session_include, key, GINT_TO_POINTER(TRUE));
}

static void
enchant_session_remove (EnchantSession * session, const char * const word, size_t len)
{
	char* key = g_strndup (word, len);
	g_hash_table_remove (session->session_include, key);
	g_hash_table_insert (session->session_exclude, key, GINT_TO_POINTER(TRUE));
}

static void
enchant_session_add_personal (EnchantSession * session, const char * const word, size_t len)
{
}

static void
enchant_session_remove_personal (EnchantSession * session, const char * const word, size_t len)
{
}

static void
enchant_session_add_exclude (EnchantSession * session, const char * const word, size_t len)
{
}

static void
enchant_session_remove_exclude (EnchantSession * session, const char * const word, size_t len)
{
}

/* a word is excluded if it is in the exclude dictionary or in the session exclude list
 *  AND the word has not been added to the session include list
 */
static gboolean
enchant_session_exclude (EnchantSession * session, const char * const word, size_t len)
{
	gboolean result = FALSE;

	char * utf = g_strndup (word, len);

	g_free (utf);

	return result;
}

static gboolean
enchant_session_contains (EnchantSession * session, const char * const word, size_t len)
{
	gboolean result = FALSE;

	char * utf = g_strndup (word, len);

	g_free (utf);

	return result;
}

static void
enchant_session_clear_error (EnchantSession * session)
{
	if (session->error)
		{
			g_free (session->error);
			session->error = NULL;
		}
}

/********************************************************************************/
/********************************************************************************/

/* @suggs must have at least n_suggs + n_new_suggs space allocated
 * @n_suggs is the number if items currently appearing in @suggs
 *
 * returns the number of items in @suggs after merge is complete
 */
static int
enchant_dict_merge_suggestions(char ** suggs, size_t n_suggs, char ** new_suggs, size_t n_new_suggs)
{
	size_t i, j;

	for(i = 0; i < n_new_suggs; i++)
		{
			int is_duplicate = 0;
			char * normalized_new_sugg;

			normalized_new_sugg = g_utf8_normalize (new_suggs[i], -1, G_NORMALIZE_NFD);

			for(j = 0; j < n_suggs; j++)
				{
					char* normalized_sugg;
					normalized_sugg = g_utf8_normalize (suggs[j], -1, G_NORMALIZE_NFD);

					if(strcmp(normalized_sugg,normalized_new_sugg)==0)
						{
							is_duplicate = 1;
							g_free(normalized_sugg);
							break;
						}
					g_free(normalized_sugg);
				}
			g_free(normalized_new_sugg);

			if(!is_duplicate)
				{
					suggs[n_suggs] = strdup (new_suggs[i]);
					++n_suggs;
				}
		}

	return n_suggs;
}

/***********************************************************************************/
/***********************************************************************************/

static void
enchant_broker_clear_error (EnchantBroker * broker)
{
	if (broker->error)
		{
			g_free (broker->error);
			broker->error = NULL;
		}
}

static void
enchant_broker_set_error (EnchantBroker * broker, const char * const err)
{
	enchant_broker_clear_error (broker);
	broker->error = strdup (err);
}

static void
enchant_load_ordering_from_file (EnchantBroker * broker, const char * file)
{
	char line[1024];

	FILE * f = g_fopen (file, "r");
	if (!f)
		return;

	while (NULL != fgets (line, sizeof(line), f)) {
		size_t i, len;

		for (i = 0, len = strlen(line); i < len && line[i] != ':'; i++)
			;

		if (i < len)
			{
				char * tag = g_strndup (line, i);
				char * ordering = g_strndup (line + (i + 1), len - i);

				enchant_broker_set_ordering (broker, tag, ordering);

				g_free (tag);
				g_free (ordering);
			}
	}

	fclose (f);
}

/**
 * enchant_broker_init
 *
 * Returns: A new broker object capable of requesting
 * dictionaries from providers.
 */
ENCHANT_MODULE_EXPORT (EnchantBroker *)
enchant_broker_init (void)
{
	EnchantBroker *broker = NULL;

	g_return_val_if_fail (g_module_supported (), NULL);

	broker = g_new0 (EnchantBroker, 1);

	return broker;
}

/**
 * enchant_broker_free
 * @broker: A non-null #EnchantBroker
 *
 * Destroys the broker object. Must only be called once per broker init
 */
ENCHANT_MODULE_EXPORT (void)
enchant_broker_free (EnchantBroker * broker)
{
	guint n_remaining;

	g_return_if_fail (broker);

	n_remaining = g_hash_table_size (broker->dict_map);
	if (n_remaining)
		{
			g_warning ("%u dictionaries weren't free'd.\n", n_remaining);
		}

	/* will destroy any remaining dictionaries for us */
	g_hash_table_destroy (broker->dict_map);

	enchant_broker_clear_error (broker);

	g_free (broker);
}

/**
 * enchant_broker_describe
 * @broker: A non-null #EnchantBroker
 * @fn: A non-null #EnchantBrokerDescribeFn
 * @user_data: Optional user-data
 *
 * Enumerates the Enchant providers and tells
 * you some rudimentary information about them.
 */
ENCHANT_MODULE_EXPORT (void)
enchant_broker_describe (EnchantBroker * broker,
			 EnchantBrokerDescribeFn fn,
			 void * user_data)
{
	GSList *list;
	GModule *module;

	const char * name, * desc, * file;

	g_return_if_fail (broker);
	g_return_if_fail (fn);

	enchant_broker_clear_error (broker);
}

/**
 * enchant_broker_list_dicts
 * @broker: A non-null #EnchantBroker
 * @fn: A non-null #EnchantDictDescribeFn
 * @user_data: Optional user-data
 *
 * Enumerates the dictionaries available from
 * all Enchant providers.
 */
ENCHANT_MODULE_EXPORT (void)
enchant_broker_list_dicts (EnchantBroker * broker,
			   EnchantDictDescribeFn fn,
			   void * user_data)
{
	GSList *list;
	GHashTable *tags;
	GHashTableIter iter;
	gpointer key, value;

	g_return_if_fail (broker);
	g_return_if_fail (fn);

	tags = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

	enchant_broker_clear_error (broker);

	g_hash_table_iter_init (&iter, tags);
	g_hash_table_destroy (tags);
}

static int
_enchant_broker_dict_exists (EnchantBroker * broker,
				 const char * const tag)
{
	GSList * list;

	/* don't query the providers if it is an empty string */
	if (tag == NULL || *tag == '\0') {
		return 0;
	}

	/* don't query the providers if we can just do a quick map lookup */
	if (g_hash_table_lookup (broker->dict_map, (gpointer) tag) != NULL) {
		return 1;
	}

	return 0;
}

/**
 * enchant_broker_dict_exists
 * @broker: A non-null #EnchantBroker
 * @tag: The non-null language tag you wish to request a dictionary for ("en_US", "de_DE", ...)
 *
 * Return existance of the requested dictionary (1 == true, 0 == false)
 */
ENCHANT_MODULE_EXPORT (int)
enchant_broker_dict_exists (EnchantBroker * broker,
				const char * const tag)
{
	char * normalized_tag;
	int exists = 0;

	g_return_val_if_fail (broker, 0);
	g_return_val_if_fail (tag && strlen(tag), 0);

	enchant_broker_clear_error (broker);

	normalized_tag = enchant_normalize_dictionary_tag (tag);

	if(!enchant_is_valid_dictionary_tag(normalized_tag))
		{
			enchant_broker_set_error (broker, "invalid tag character found");
		}
	else if ((exists = _enchant_broker_dict_exists (broker, normalized_tag)) == 0)
		{
			char * iso_639_only_tag;

			iso_639_only_tag = enchant_iso_639_from_tag (normalized_tag);

			if (strcmp (normalized_tag, iso_639_only_tag) != 0)
				{
					exists = _enchant_broker_dict_exists (broker, iso_639_only_tag);
				}

			free (iso_639_only_tag);
		}

	free (normalized_tag);
	return exists;
}

/**
 * enchant_broker_set_ordering
 * @broker: A non-null #EnchantBroker
 * @tag: A non-null language tag (en_US)
 * @ordering: A non-null ordering (aspell,hunspell,uspell,hspell)
 *
 * Declares a preference of dictionaries to use for the language
 * described/referred to by @tag. The ordering is a comma delimited
 * list of provider names. As a special exception, the "*" tag can
 * be used as a language tag to declare a default ordering for any
 * language that does not explictly declare an ordering.
 */
ENCHANT_MODULE_EXPORT (void)
enchant_broker_set_ordering (EnchantBroker * broker,
				 const char * const tag,
				 const char * const ordering)
{
	char * tag_dupl;
	char * ordering_dupl;

	g_return_if_fail (broker);
	g_return_if_fail (tag && strlen(tag));
	g_return_if_fail (ordering && strlen(ordering));

	enchant_broker_clear_error (broker);

	tag_dupl = enchant_normalize_dictionary_tag (tag);

	ordering_dupl = g_strdup (ordering);
	ordering_dupl = g_strstrip (ordering_dupl);
}

/**
 * enchant_broker_get_error
 * @broker: A non-null broker
 *
 * Returns a const char string or NULL describing the last exception in UTF8 encoding.
 * WARNING: error is transient and is likely cleared as soon as the
 * next broker operation happens
 */
ENCHANT_MODULE_EXPORT(char *)
enchant_broker_get_error (EnchantBroker * broker)
{
	g_return_val_if_fail (broker, NULL);

	return broker->error;
}

/**
 * enchant_get_user_language
 *
 * Returns a char string giving the current language.
 * Defaults to "en" if no language or locale can be found, or
 * locale is C.
 *
 * The returned string should be free'd with free.
 */
ENCHANT_MODULE_EXPORT(char *)
enchant_get_user_language(void)
{
	const char * locale = NULL;

#if defined(G_OS_WIN32)
	if(!locale)
		{ /* Copy locale string so it does not need to be freed */
			char * win_locale = g_win32_getlocale ();
			locale = alloca (strlen (win_locale));
			strcpy (locale, win_locale);
			g_free (win_locale);
		}
#endif

	if(!locale)
		locale = g_getenv ("LANG");

#if defined(HAVE_LC_MESSAGES)
	if(!locale)
		locale = setlocale (LC_MESSAGES, NULL);
#endif

	if(!locale)
		locale = setlocale (LC_ALL, NULL);

	if(!locale || strcmp(locale, "C") == 0)
		locale = "en";

	return strdup (locale);
}

/**
 * enchant_set_prefix_dir
 *
 * Set the prefix dir. This overrides any auto-detected value,
 * and can also be used on systems or installations where
 * auto-detection does not work.
 *
 */
ENCHANT_MODULE_EXPORT (void)
enchant_set_prefix_dir(const char *new_prefix)
{
	set_relocation_prefix (INSTALLPREFIX, new_prefix);
}

ENCHANT_MODULE_EXPORT(const char *) _GL_ATTRIBUTE_CONST
enchant_get_version (void) {
	return "";
}
