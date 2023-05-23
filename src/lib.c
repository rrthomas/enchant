/* enchant
 * Copyright (C) 2003, 2004 Dom Lachowicz
 * Copyright (C) 2016-2023 Reuben Thomas <rrt@sc3d.org>
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <glib.h>
#include <gmodule.h>
#include <glib/gstdio.h>
#include <locale.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "enchant.h"
#include "enchant-provider.h"
#include "debug.h"
#include "pwl.h"
#include "relocatable.h"
#include "configmake.h"

/********************************************************************************/

struct str_enchant_broker
{
	GSList *provider_list;	/* list of all of the spelling backend providers */
	GHashTable *dict_map;		/* map of language tag -> dictionary */
	GHashTable *provider_ordering; /* map of language tag -> provider order */

	gchar * error;
};

typedef struct str_enchant_session
{
	GHashTable *session_include;
	GHashTable *session_exclude;
	EnchantPWL *personal;
	EnchantPWL *exclude;

	char * personal_filename;
	char * exclude_filename;
	char * language_tag;

	char * error;

	gboolean is_pwl;

	EnchantProvider * provider;
} EnchantSession;

typedef struct str_enchant_dict_private_data
{
	unsigned int reference_count;
	EnchantSession* session;
} EnchantDictPrivateData;

typedef EnchantProvider *(*EnchantProviderInitFunc) (void);
typedef void             (*EnchantPreConfigureFunc) (EnchantProvider * provider, const char * module_dir);

/********************************************************************************/
/********************************************************************************/

/* Relocate a path and ensure the result is allocated on the heap */
char *
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

_GL_ATTRIBUTE_MALLOC char *
enchant_get_user_config_dir (void)
{
	const gchar * env = g_getenv("ENCHANT_CONFIG_DIR");
	if (env)
		return g_filename_to_utf8(env, -1, NULL, NULL, NULL);
	return g_build_filename (g_get_user_config_dir (), "enchant", NULL);
}

GSList *
enchant_get_conf_dirs (void)
{
	GSList *conf_dirs = NULL;
	char *pkgdatadir = NULL;
	char *sysconfdir = NULL;
	char *pkgconfdir = NULL;
	char *user_config_dir = NULL;

	if ((pkgdatadir = enchant_relocate (PKGDATADIR)) == NULL)
		goto error_exit;
	conf_dirs = g_slist_append (conf_dirs, pkgdatadir);

	if ((sysconfdir = enchant_relocate (SYSCONFDIR)) == NULL)
		goto error_exit;
	if ((pkgconfdir = g_build_filename (sysconfdir, "enchant", NULL)) == NULL)
		goto error_exit;
	conf_dirs = g_slist_append (conf_dirs, pkgconfdir);
	free (sysconfdir);
	sysconfdir = NULL;

	if ((user_config_dir = enchant_get_user_config_dir ()) == NULL)
		goto error_exit;
	conf_dirs = g_slist_append (conf_dirs, user_config_dir);

	return conf_dirs;

 error_exit:
	free (pkgdatadir);
	free (sysconfdir);
	g_free (pkgconfdir);
	free (user_config_dir);
	return NULL;
}

/********************************************************************************/
/********************************************************************************/

/* returns TRUE if tag is valid
 * for requires alphanumeric ASCII or underscore
 */
static G_GNUC_PURE int
enchant_is_valid_dictionary_tag(const char * const tag)
{
	const char * it;
	for (it = tag; *it; ++it)
		if(!g_ascii_isalnum(*it) && *it != '_')
			return 0;

	return it != tag; /*empty tag invalid*/
}

static char *
enchant_normalize_dictionary_tag (const char * const dict_tag)
{
	char * new_tag = g_strstrip (strdup (dict_tag));

	/* strip off en_GB@euro */
	*strchrnul (new_tag, '@') = '\0';

	/* strip off en_GB.UTF-8 */
	*strchrnul (new_tag, '.') = '\0';

	/* turn en-GB into en_GB */
	char * needle;
	if ((needle = strchr (new_tag, '-')) != NULL)
		*needle = '_';

	/* everything before first '_' is converted to lower case */
	needle = strchrnul (new_tag, '_');
	for (gchar *it = new_tag; it != needle; ++it)
		*it = g_ascii_tolower (*it);
	/* everything after first '_' is converted to upper case */
	for (gchar *it = needle; *it; ++it)
		*it = g_ascii_toupper (*it);

	return new_tag;
}

static char *
enchant_iso_639_from_tag (const char * const dict_tag)
{
	char * new_tag = strdup (dict_tag);
	char * needle = strchr (new_tag, '_');

	if (needle != NULL)
		*needle = '\0';

	return new_tag;
}

static void
enchant_session_destroy (EnchantSession * session)
{
	g_hash_table_destroy (session->session_include);
	g_hash_table_destroy (session->session_exclude);
	enchant_pwl_free (session->personal);
	enchant_pwl_free (session->exclude);
	g_free (session->personal_filename);
	g_free (session->exclude_filename);
	free (session->language_tag);

	if (session->error)
		g_free (session->error);

	g_free (session);
}

static EnchantSession *
enchant_session_new_with_pwl (EnchantProvider * provider,
			      const char * const pwl,
			      const char * const excl,
			      const char * const lang,
			      gboolean fail_if_no_pwl)
{
	EnchantPWL *personal = NULL;
	if (pwl)
		personal = enchant_pwl_init_with_file (pwl);
	if (personal == NULL) {
		if (fail_if_no_pwl)
			return NULL;
		else
			personal = enchant_pwl_init ();
	}

	EnchantPWL *exclude = NULL;
	if (excl)
		exclude = enchant_pwl_init_with_file (excl);
	if (exclude == NULL)
		exclude = enchant_pwl_init ();

	EnchantSession * session = g_new0 (EnchantSession, 1);
	session->session_include = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	session->session_exclude = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	session->personal = personal;
	session->exclude = exclude;
	session->provider = provider;
	session->language_tag = strdup (lang);
	session->personal_filename = g_strdup (pwl); /* Need g_strdup because may be NULL */
	session->exclude_filename = g_strdup (excl); /* Need g_strdup because may be NULL */

	return session;
}

static EnchantSession *
enchant_session_new (EnchantProvider *provider, const char * const lang, const char * pwl)
{
	char *user_config_dir = enchant_get_user_config_dir ();

	EnchantSession * session = NULL;
	if (!user_config_dir || !lang)
		return NULL;
	enchant_ensure_dir_exists (user_config_dir);

	char *filename, *excl = NULL, *alloc_pwl = NULL;
	if (pwl == NULL) {
		filename = g_strdup_printf ("%s.dic", lang);
		pwl = alloc_pwl = g_build_filename (user_config_dir, filename, NULL);
		g_free (filename);

		filename = g_strdup_printf ("%s.exc", lang);
		excl = g_build_filename (user_config_dir, filename, NULL);
		g_free (filename);
	}

	// Insist that PWL be opened only if we supplied it explicitly.
	session = enchant_session_new_with_pwl (provider, pwl, excl, lang, alloc_pwl == NULL);

	g_free (alloc_pwl);
	g_free (excl);
	g_free (user_config_dir);

	return session;
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

/* a word is excluded if it is in the exclude dictionary or in the session exclude list
 *  AND the word has not been added to the session include list
 */
static gboolean
enchant_session_exclude (EnchantSession * session, const char * const word, size_t len)
{
	char * utf = g_strndup (word, len);
	gboolean result = !g_hash_table_lookup (session->session_include, utf) &&
		(g_hash_table_lookup (session->session_exclude, utf) ||
		 enchant_pwl_check (session->exclude, word, len) == 0);
	g_free (utf);

	return result;
}

static gboolean
enchant_session_contains (EnchantSession * session, const char * const word, size_t len)
{
	char * utf = g_strndup (word, len);
	gboolean result = g_hash_table_lookup (session->session_include, utf) ||
		(enchant_pwl_check (session->personal, word, len) == 0 &&
		 (!enchant_pwl_check (session->exclude, word, len)) == 0);
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

static void
enchant_free_string_list (char ** string_list)
{
	g_strfreev (string_list);
}

void
enchant_dict_set_error (EnchantDict * dict, const char * const err)
{
	g_return_if_fail (dict);
	g_return_if_fail (err);
	g_return_if_fail (g_utf8_validate(err, -1, NULL));

	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);
	session->error = strdup (err);
}

const char *
enchant_dict_get_error (EnchantDict * dict)
{
	g_return_val_if_fail (dict, NULL);

	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	return session->error;
}

int
enchant_dict_check (EnchantDict * dict, const char *const word, ssize_t len)
{
	g_return_val_if_fail (dict, -1);
	g_return_val_if_fail (word, -1);

	if (len < 0)
		len = strlen (word);

	g_return_val_if_fail (len, -1);
	g_return_val_if_fail (g_utf8_validate(word, len, NULL),-1);

	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	/* first, see if it's to be excluded*/
	if (enchant_session_exclude (session, word, len))
		return 1;

	/* then, see if it's in our pwl or session*/
	if (enchant_session_contains(session, word, len))
		return 0;

	if (dict->check)
		return (*dict->check) (dict, word, len);
	else if (session->is_pwl)
		return 1;

	return -1;
}

/* @suggs must have at least n_suggs + n_new_suggs space allocated
 * @n_suggs is the number if items currently appearing in @suggs
 *
 * returns the number of items in @suggs after merge is complete
 */
static int
enchant_dict_merge_suggestions(char ** suggs, size_t n_suggs, char ** new_suggs, size_t n_new_suggs)
{
	for (size_t i = 0; i < n_new_suggs; i++)
		{
			char * normalized_new_sugg = g_utf8_normalize (new_suggs[i], -1, G_NORMALIZE_NFD);

			int is_duplicate = 0;
			for (size_t j = 0; !is_duplicate && j < n_suggs; j++)
				{
					char* normalized_sugg = g_utf8_normalize (suggs[j], -1, G_NORMALIZE_NFD);
					is_duplicate = strcmp (normalized_sugg, normalized_new_sugg) == 0;
					g_free (normalized_sugg);
				}
			g_free (normalized_new_sugg);

			if (!is_duplicate)
				suggs[n_suggs++] = strdup (new_suggs[i]);
		}

	return n_suggs;
}

static char **
enchant_dict_get_good_suggestions(EnchantDict * dict, char ** suggs, size_t n_suggs, size_t* out_n_filtered_suggs)
{
	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;

	char ** filtered_suggs = g_new0 (char *, n_suggs + 1);
	size_t n_filtered_suggs = 0;
	for (size_t i = 0; i < n_suggs; i++)
		{
			size_t sugg_len = strlen(suggs[i]);

			if (sugg_len == 0)
				continue;

			if (g_utf8_validate(suggs[i], sugg_len, NULL) &&
			    !enchant_session_exclude(session, suggs[i], sugg_len))
				filtered_suggs[n_filtered_suggs++] = strdup (suggs[i]);
		}

	if (out_n_filtered_suggs)
		*out_n_filtered_suggs = n_filtered_suggs;

	return filtered_suggs;
}

char **
enchant_dict_suggest (EnchantDict * dict, const char *const word, ssize_t len, size_t * out_n_suggs)
{
	g_return_val_if_fail (dict, NULL);
	g_return_val_if_fail (word, NULL);

	if (len < 0)
		len = strlen (word);

	g_return_val_if_fail (len, NULL);
	g_return_val_if_fail (g_utf8_validate(word, len, NULL), NULL);

	size_t n_dict_suggs = 0, n_suggsT = 0;
	char **dict_suggs = NULL, **suggsT;

	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	/* Check for suggestions from provider dictionary */
	if (dict->suggest)
		{
			dict_suggs = (*dict->suggest) (dict, word, len, &n_dict_suggs);
			if (dict_suggs)
				{
					suggsT = enchant_dict_get_good_suggestions(dict, dict_suggs, n_dict_suggs, &n_suggsT);
					enchant_free_string_list (dict_suggs);
					dict_suggs = suggsT;
					n_dict_suggs = n_suggsT;
				}
		}

	/* Clone suggestions, if any */
	char **suggs = NULL;
	size_t n_suggs = n_dict_suggs;
	if (n_suggs > 0)
		{
			suggs = g_new0 (char *, n_suggs + 1);
			n_suggs = 0;
			if (dict_suggs != NULL)
				n_suggs = enchant_dict_merge_suggestions(suggs, n_suggs, dict_suggs, n_dict_suggs);
		}

	g_strfreev(dict_suggs);

	if (out_n_suggs)
		*out_n_suggs = n_suggs;

	return suggs;
}

void
enchant_dict_add (EnchantDict * dict, const char *const word, ssize_t len)
{
	g_return_if_fail (dict);
	g_return_if_fail (word);

	if (len < 0)
		len = strlen (word);

	g_return_if_fail (len);
	g_return_if_fail (g_utf8_validate(word, len, NULL));

	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);
	enchant_pwl_add(session->personal, word, len);
	enchant_pwl_remove(session->exclude, word, len);

	if (dict->add_to_personal)
		(*dict->add_to_personal) (dict, word, len);
}

void
enchant_dict_add_to_session (EnchantDict * dict, const char *const word, ssize_t len)
{
	g_return_if_fail (dict);
	g_return_if_fail (word);

	if (len < 0)
		len = strlen (word);

	g_return_if_fail (len);
	g_return_if_fail (g_utf8_validate(word, len, NULL));

	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	enchant_session_add (session, word, len);
	if (dict->add_to_session)
		(*dict->add_to_session) (dict, word, len);
}

int
enchant_dict_is_added (EnchantDict * dict, const char *const word, ssize_t len)
{
	g_return_val_if_fail (dict, 0);
	g_return_val_if_fail (word, 0);

	if (len < 0)
		len = strlen (word);

	g_return_val_if_fail (len, 0);
	g_return_val_if_fail (g_utf8_validate(word, len, NULL), 0);

	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	return enchant_session_contains (session, word, len);
}

void
enchant_dict_remove (EnchantDict * dict, const char *const word, ssize_t len)
{
	g_return_if_fail (dict);
	g_return_if_fail (word);

	if (len < 0)
		len = strlen (word);

	g_return_if_fail (len);
	g_return_if_fail (g_utf8_validate(word, len, NULL));

	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	enchant_pwl_remove(session->personal, word, len);
	enchant_pwl_add(session->exclude, word, len);

	if (dict->add_to_exclude)
		(*dict->add_to_exclude) (dict, word, len);
}

void
enchant_dict_remove_from_session (EnchantDict * dict, const char *const word, ssize_t len)
{
	g_return_if_fail (dict);
	g_return_if_fail (word);

	if (len < 0)
		len = strlen (word);

	g_return_if_fail (len);
	g_return_if_fail (g_utf8_validate(word, len, NULL));

	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	enchant_session_remove (session, word, len);
	if (dict->remove_from_session)
		(*dict->remove_from_session) (dict, word, len);
}

int
enchant_dict_is_removed (EnchantDict * dict, const char *const word, ssize_t len)
{
	g_return_val_if_fail (dict, 0);
	g_return_val_if_fail (word, 0);

	if (len < 0)
		len = strlen (word);

	g_return_val_if_fail (len, 0);
	g_return_val_if_fail (g_utf8_validate(word, len, NULL), 0);

	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	return enchant_session_exclude (session, word, len);
}

void
enchant_dict_store_replacement (EnchantDict * dict,
				const char *const mis, ssize_t mis_len,
				const char *const cor, ssize_t cor_len)
{
	g_return_if_fail (dict);
	g_return_if_fail (mis);
	g_return_if_fail (cor);

	if (mis_len < 0)
		mis_len = strlen (mis);

	if (cor_len < 0)
		cor_len = strlen (cor);

	g_return_if_fail (mis_len);
	g_return_if_fail (cor_len);

	g_return_if_fail (g_utf8_validate(mis, mis_len, NULL));
	g_return_if_fail (g_utf8_validate(cor, cor_len, NULL));

	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	/* if it's not implemented, it's not worth emulating */
	if (dict->store_replacement)
		(*dict->store_replacement) (dict, mis, mis_len, cor, cor_len);
}

void
enchant_dict_free_string_list (EnchantDict * dict, char **string_list)
{
	g_return_if_fail (dict);

	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);
	g_strfreev(string_list);
}

void
enchant_dict_describe (EnchantDict * dict, EnchantDictDescribeFn fn, void * user_data)
{
	g_return_if_fail (dict);
	g_return_if_fail (fn);

	EnchantSession * session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);
	EnchantProvider * provider = session->provider;

	const char * name, * desc, * file;
	if (provider)
		{
			GModule *module = (GModule *) provider->enchant_private_data;
			file = g_module_name (module);
			name = (*provider->identify) (provider);
			desc = (*provider->describe) (provider);
		}
	else
		{
			file = session->personal_filename;
			name = "Personal Wordlist";
			desc = "Personal Wordlist";
		}

	const char *tag = session->language_tag;
	(*fn) (tag, name, desc, file, user_data);
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

static int
enchant_provider_is_valid(EnchantProvider * provider)
{
	if (provider == NULL)
		g_warning ("EnchantProvider cannot be NULL\n");
	else if (provider->dispose == NULL)
		g_warning ("EnchantProvider's dispose method cannot be NULL\n");
	else if (provider->request_dict == NULL)
		g_warning ("EnchantProvider's request_dict method cannot be NULL\n");
	else if (provider->dispose_dict == NULL)
		g_warning ("EnchantProvider's dispose_dict method cannot be NULL\n");
	else if (provider->identify == NULL)
		g_warning ("EnchantProvider's identify method cannot be NULL\n");
	else if (!g_utf8_validate((*provider->identify)(provider), -1, NULL))
		g_warning ("EnchantProvider's identify method does not return valid UTF-8\n");
	else if (provider->describe == NULL)
		g_warning ("EnchantProvider's describe method cannot be NULL\n");
	else if (!g_utf8_validate((*provider->describe)(provider), -1, NULL))
		g_warning ("EnchantProvider's describe method does not return valid UTF-8\n");
	else if (provider->list_dicts == NULL)
		g_warning ("EnchantProvider's list_dicts method cannot be NULL\n");
	else
		return 1;

	return 0;
}

static void
enchant_load_providers_in_dir (EnchantBroker * broker, const char *dir_name)
{
	GDir *dir = g_dir_open (dir_name, 0, NULL);
	if (!dir)
		return;

	size_t g_module_suffix_len = strlen (G_MODULE_SUFFIX);
	const char *dir_entry;
	while ((dir_entry = g_dir_read_name (dir)) != NULL)
		{
			GModule *module = NULL;
			EnchantProvider *provider = NULL;

			size_t entry_len = strlen (dir_entry);
			if ((entry_len > g_module_suffix_len) &&
				dir_entry[0] != '.' && /* Skip hidden files */
				!strcmp(dir_entry+(entry_len-g_module_suffix_len), G_MODULE_SUFFIX))
				{
#ifdef _WIN32
					/* Suppress error popups for failing to load plugins */
					UINT old_error_mode = SetErrorMode(SEM_FAILCRITICALERRORS);
#endif
					char * filename = g_build_filename (dir_name, dir_entry, NULL);
					module = g_module_open (filename, (GModuleFlags) 0);
					if (module)
						{
							EnchantProviderInitFunc init_func;
							if (g_module_symbol (module, "init_enchant_provider", (gpointer *) (&init_func))
							    && init_func)
								{
									provider = init_func ();
									if (!enchant_provider_is_valid(provider))
										{
											g_warning ("Error loading plugin: %s's init_enchant_provider returned invalid provider.\n", dir_entry);
											if(provider)
												{
													provider->dispose(provider);
													provider = NULL;
												}
											g_module_close (module);
										}
								}
							else
								g_module_close (module);
						}
					else
						g_warning ("Error loading plugin: %s\n", g_module_error());

					g_free (filename);
#ifdef _WIN32
					/* Restore the original error mode */
					SetErrorMode(old_error_mode);
#endif
				}
			if (provider)
				{
					/* optional entry point to allow modules to look for associated files */
					EnchantPreConfigureFunc conf_func;
					if (g_module_symbol (module, "configure_enchant_provider", (gpointer *) (&conf_func))
					    && conf_func)
						{
							conf_func (provider, dir_name);
							if (!enchant_provider_is_valid(provider))
								{
									g_warning ("Error loading plugin: %s's configure_enchant_provider modified provider and it is now invalid.\n", dir_entry);
									provider->dispose(provider);
									provider = NULL;
									g_module_close (module);
								}
						}
				}
			if (provider)
				{
					provider->enchant_private_data = (void *) module;
					provider->owner = broker;
					broker->provider_list = g_slist_append (broker->provider_list, (gpointer)provider);
				}
		}

	g_dir_close (dir);
}

static void
enchant_load_providers (EnchantBroker * broker)
{
	char *module_dir = enchant_relocate (PKGLIBDIR "-" ENCHANT_MAJOR_VERSION);
	if (module_dir)
		enchant_load_providers_in_dir (broker, module_dir);
	free (module_dir);
}

static void
enchant_load_ordering_from_file (EnchantBroker * broker, const char * file)
{
	GIOChannel * ch = g_io_channel_new_file (file, "r", NULL);
	if (!ch)
		return;

	gchar *line;
	gsize terminator;
	while (G_IO_STATUS_NORMAL == g_io_channel_read_line (ch, &line, NULL, &terminator, NULL)) {
		char *colon = strchr (line, ':');
		if (colon != NULL)
			{
				char * tag = g_strndup (line, colon - line);
				char * ordering = g_strndup (colon + 1, terminator - 1);
				enchant_broker_set_ordering (broker, tag, ordering);

				g_free (tag);
				g_free (ordering);
			}
		g_free (line);
	}

	g_io_channel_unref (ch);
}

static void
enchant_load_provider_ordering (EnchantBroker * broker)
{
	broker->provider_ordering = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

	GSList *conf_dirs = enchant_get_conf_dirs ();
	for (GSList *iter = conf_dirs; iter; iter = iter->next)
		{
			char *ordering_file = g_build_filename (iter->data, "enchant.ordering", NULL);
			enchant_load_ordering_from_file (broker, ordering_file);
			g_free (ordering_file);
		}

	g_slist_free_full (conf_dirs, g_free);
}

static GSList *
enchant_get_ordered_providers (EnchantBroker * broker, const char * const tag)
{
	char * ordering = (char *)g_hash_table_lookup (broker->provider_ordering, (gpointer)tag);
	if (!ordering)
		ordering = (char *)g_hash_table_lookup (broker->provider_ordering, (gpointer)"*");

	GSList * list = NULL;

	if (ordering)
		{
			char **tokens = g_strsplit (ordering, ",", 0);
			if (tokens)
				{
					for (size_t i = 0; tokens[i]; i++)
						{
							char *token = g_strstrip(tokens[i]);

							for (GSList * iter = broker->provider_list; iter != NULL; iter = g_slist_next (iter))
								{
									EnchantProvider *provider = (EnchantProvider*)iter->data;
									if (provider && !strcmp (token, (*provider->identify)(provider)))
										list = g_slist_append (list, (gpointer)provider);
								}
						}
					g_strfreev (tokens);
				}
		}

	/* append providers not in the list, or from an unordered list */
	for (GSList * iter = broker->provider_list; iter != NULL; iter = g_slist_next (iter))
		if (!g_slist_find (list, iter->data))
			list = g_slist_append (list, iter->data);

	return list;
}

static void
enchant_dict_destroyed (gpointer data)
{
	g_return_if_fail (data);

	EnchantDict *dict = (EnchantDict *) data;
	EnchantDictPrivateData *enchant_dict_private_data = (EnchantDictPrivateData*)dict->enchant_private_data;
	EnchantSession *session = enchant_dict_private_data->session;
	EnchantProvider *owner = session->provider;

	if (owner)
		(*owner->dispose_dict) (owner, dict);
	else if(session->is_pwl)
		g_free (dict);

	g_free(enchant_dict_private_data);

	enchant_session_destroy (session);
}

static void
enchant_provider_free (gpointer data)
{
	g_return_if_fail (data);

	EnchantProvider *provider = (EnchantProvider *) data;
	GModule *module = (GModule *) provider->enchant_private_data;
	(*provider->dispose) (provider);

	/* close module only after invoking dispose */
	g_module_close (module);
}

EnchantBroker *
enchant_broker_init (void)
{
	g_return_val_if_fail (g_module_supported (), NULL);

	EnchantBroker *broker = g_new0 (EnchantBroker, 1);
	broker->dict_map = g_hash_table_new_full (g_str_hash, g_str_equal,
						  g_free, enchant_dict_destroyed);
	enchant_load_providers (broker);
	enchant_load_provider_ordering (broker);

	return broker;
}

void
enchant_broker_free (EnchantBroker * broker)
{
	g_return_if_fail (broker);

	guint n_remaining = g_hash_table_size (broker->dict_map);
	if (n_remaining)
		g_warning ("%u dictionaries weren't free'd.\n", n_remaining);

	/* will destroy any remaining dictionaries for us */
	g_hash_table_destroy (broker->dict_map);
	g_hash_table_destroy (broker->provider_ordering);

	g_slist_free_full (broker->provider_list, enchant_provider_free);
	enchant_broker_clear_error (broker);
	g_free (broker);
}

EnchantDict *
enchant_broker_request_pwl_dict (EnchantBroker * broker, const char *const pwl)
{
	g_return_val_if_fail (broker, NULL);
	g_return_val_if_fail (pwl && strlen(pwl), NULL);

	enchant_broker_clear_error (broker);

	EnchantDict *dict = (EnchantDict*)g_hash_table_lookup (broker->dict_map, (gpointer) pwl);
	if (dict) {
		((EnchantDictPrivateData*)dict->enchant_private_data)->reference_count++;
		return dict;
	}

	/* since the broker pwl file is a read/write file (there is no readonly dictionary associated)
	 * there is no need for complementary exclude file to add a word to. The word just needs to be
	 * removed from the broker pwl file
	 */
	EnchantSession *session = enchant_session_new_with_pwl (NULL, pwl, NULL, "Personal Wordlist", TRUE);
	if (!session)
		{
			broker->error = g_strdup_printf ("Couldn't open personal wordlist '%s'", pwl);
			return NULL;
		}

	session->is_pwl = 1;

	dict = g_new0 (EnchantDict, 1);
	EnchantDictPrivateData *enchant_dict_private_data = g_new0 (EnchantDictPrivateData, 1);
	enchant_dict_private_data->reference_count = 1;
	enchant_dict_private_data->session = session;
	dict->enchant_private_data = (void *)enchant_dict_private_data;

	g_hash_table_insert (broker->dict_map, (gpointer)strdup (pwl), dict);

	return dict;
}

static EnchantDict *
_enchant_broker_request_dict (EnchantBroker * broker, const char *const tag, const char *const pwl)
{
	EnchantDict *dict = (EnchantDict*)g_hash_table_lookup (broker->dict_map, (gpointer) tag);
	if (dict) {
		((EnchantDictPrivateData*)dict->enchant_private_data)->reference_count++;
		return dict;
	}

	GSList * list = enchant_get_ordered_providers (broker, tag);
	for (GSList *listIter = list; listIter != NULL; listIter = g_slist_next (listIter))
		{
			EnchantProvider * provider = (EnchantProvider *) listIter->data;

			dict = (*provider->request_dict) (provider, tag);

			if (dict)
				{
					EnchantSession *session = enchant_session_new (provider, tag, pwl);
					EnchantDictPrivateData *enchant_dict_private_data = g_new0 (EnchantDictPrivateData, 1);
					enchant_dict_private_data->reference_count = 1;
					enchant_dict_private_data->session = session;
					dict->enchant_private_data = (void *)enchant_dict_private_data;
					g_hash_table_insert (broker->dict_map, (gpointer)strdup (tag), dict);
					break;
				}
		}
	g_slist_free (list);

	return dict;
}

EnchantDict *
enchant_broker_request_dict_with_pwl (EnchantBroker * broker, const char *const tag, const char *const pwl)
{
	EnchantDict *dict = NULL;

	g_return_val_if_fail (broker, NULL);
	g_return_val_if_fail (tag && strlen(tag), NULL);

	enchant_broker_clear_error (broker);

	char * normalized_tag = enchant_normalize_dictionary_tag (tag);
	if(!enchant_is_valid_dictionary_tag(normalized_tag))
		enchant_broker_set_error (broker, "invalid tag character found");
	else if ((dict = _enchant_broker_request_dict (broker, normalized_tag, pwl)) == NULL)
		{
			char * iso_639_only_tag = enchant_iso_639_from_tag (normalized_tag);
			dict = _enchant_broker_request_dict (broker, iso_639_only_tag, pwl);
			free (iso_639_only_tag);
		}
	free (normalized_tag);

	return dict;
}

EnchantDict *
enchant_broker_request_dict (EnchantBroker * broker, const char *const tag)
{
	return enchant_broker_request_dict_with_pwl (broker, tag, NULL);
}

void
enchant_broker_describe (EnchantBroker * broker, EnchantBrokerDescribeFn fn, void * user_data)
{
	g_return_if_fail (broker);
	g_return_if_fail (fn);

	enchant_broker_clear_error (broker);

	for (GSList *list = broker->provider_list; list != NULL; list = g_slist_next (list))
		{
			EnchantProvider *provider = (EnchantProvider *) list->data;
			GModule *module = (GModule *) provider->enchant_private_data;

			const char *name = (*provider->identify) (provider);
			const char *desc = (*provider->describe) (provider);
			const char *file = g_module_name (module);

			(*fn) (name, desc, file, user_data);
		}
}

static gint
_gfunc_strcmp(gconstpointer item1, gconstpointer item2)
{
	return strcmp (item1, item2);
}

void
enchant_broker_list_dicts (EnchantBroker * broker, EnchantDictDescribeFn fn, void * user_data)
{
	g_return_if_fail (broker);
	g_return_if_fail (fn);

	GHashTable *tag_map = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

	enchant_broker_clear_error (broker);

	for (GSList *list = broker->provider_list; list != NULL; list = g_slist_next (list))
		{
			EnchantProvider *provider = (EnchantProvider *) list->data;

			size_t n_dicts;
			char ** dicts = (*provider->list_dicts) (provider, &n_dicts);

			for (size_t i = 0; i < n_dicts; i++)
				{
					const char * tag = dicts[i];
					if (enchant_is_valid_dictionary_tag (tag)) {
						GSList *providers = enchant_get_ordered_providers (broker, tag);
						gint this_priority = g_slist_index (providers, provider);
						if (this_priority != -1) {
							gint min_priority = this_priority + 1;
							gpointer ptr = g_hash_table_lookup (tag_map, tag);
							if (ptr != NULL)
								min_priority = g_slist_index (providers, ptr);
							if (this_priority < min_priority)
								g_hash_table_insert (tag_map, strdup (tag), provider);
						}
						g_slist_free (providers);
					}
				}

			enchant_free_string_list (dicts);
		}

	GSList *tags = NULL;
	GHashTableIter iter;
	g_hash_table_iter_init (&iter, tag_map);
	gpointer key, value;
	while (g_hash_table_iter_next (&iter, &key, &value))
		tags = g_slist_insert_sorted (tags, (char *) key, _gfunc_strcmp);

	for (GSList *ptr = tags; ptr != NULL; ptr = g_slist_next (ptr))
		{
			const char *tag = (const char *) ptr->data;
			EnchantProvider *provider = (EnchantProvider *) g_hash_table_lookup (tag_map, tag);
			GModule *module = (GModule *) provider->enchant_private_data;
			const char *name = (*provider->identify) (provider);
			const char *desc = (*provider->describe) (provider);
			const char *file = g_module_name (module);
			(*fn) (tag, name, desc, file, user_data);
		}

	g_slist_free (tags);
	g_hash_table_destroy (tag_map);
}

void
enchant_broker_free_dict (EnchantBroker * broker, EnchantDict * dict)
{
	g_return_if_fail (broker);
	g_return_if_fail (dict);

	enchant_broker_clear_error (broker);

	EnchantDictPrivateData * dict_private_data = (EnchantDictPrivateData*)dict->enchant_private_data;
	dict_private_data->reference_count--;
	if(dict_private_data->reference_count == 0)
		{
			EnchantSession * session = dict_private_data->session;

			if (session->provider)
				g_hash_table_remove (broker->dict_map, session->language_tag);
			else
				g_hash_table_remove (broker->dict_map, session->personal_filename);
		}
}

static int
enchant_provider_dictionary_exists (EnchantProvider * provider, const char * const tag)
{
	int exists = 0;

	if (provider->dictionary_exists)
		exists = (*provider->dictionary_exists) (provider, tag);
	else
		{
			size_t n_dicts;
			char ** dicts = (*provider->list_dicts) (provider, &n_dicts);

			for (size_t i = 0; i < n_dicts; i++)
				if (!strcmp(dicts[i], tag)) {
					exists = 1;
					break;
				}

			enchant_free_string_list (dicts);
		}

	return exists;
}

static int
_enchant_broker_dict_exists (EnchantBroker * broker, const char * const tag)
{
	/* don't query the providers if it is an empty string */
	if (tag == NULL || *tag == '\0')
		return 0;

	/* don't query the providers if we can just do a quick map lookup */
	if (g_hash_table_lookup (broker->dict_map, (gpointer) tag) != NULL)
		return 1;

	for (GSList *list = broker->provider_list; list != NULL; list = g_slist_next (list))
		if (enchant_provider_dictionary_exists ((EnchantProvider *) list->data, tag))
			return 1;

	return 0;
}

int
enchant_broker_dict_exists (EnchantBroker * broker, const char * const tag)
{
	g_return_val_if_fail (broker, 0);
	g_return_val_if_fail (tag && strlen(tag), 0);

	enchant_broker_clear_error (broker);

	char * normalized_tag = enchant_normalize_dictionary_tag (tag);
	int exists = 0;

	if(!enchant_is_valid_dictionary_tag(normalized_tag))
		enchant_broker_set_error (broker, "invalid tag character found");
	else if ((exists = _enchant_broker_dict_exists (broker, normalized_tag)) == 0)
		{
			char * iso_639_only_tag = enchant_iso_639_from_tag (normalized_tag);

			if (strcmp (normalized_tag, iso_639_only_tag) != 0)
				exists = _enchant_broker_dict_exists (broker, iso_639_only_tag);

			free (iso_639_only_tag);
		}

	free (normalized_tag);
	return exists;
}

G_GNUC_PURE const char *
enchant_dict_get_extra_word_characters (EnchantDict *dict)
{
	g_return_val_if_fail (dict, NULL);

	return dict->get_extra_word_characters ? (*dict->get_extra_word_characters) (dict) : "";
}

G_GNUC_PURE int
enchant_dict_is_word_character (EnchantDict * dict, uint32_t uc_in, size_t n)
{
	g_return_val_if_fail (n <= 2, 0);

	if (dict && dict->is_word_character)
		return (*dict->is_word_character) (dict, uc_in, n);

	gunichar uc = (gunichar)uc_in;

	/* Accept quote marks anywhere except at the end of a word */
	if (uc == g_utf8_get_char("'") || uc == g_utf8_get_char("’"))
		return n < 2;

	GUnicodeType type = g_unichar_type(uc);

	switch (type) {
	case G_UNICODE_MODIFIER_LETTER:
	case G_UNICODE_LOWERCASE_LETTER:
	case G_UNICODE_TITLECASE_LETTER:
	case G_UNICODE_UPPERCASE_LETTER:
	case G_UNICODE_OTHER_LETTER:
	case G_UNICODE_SPACING_MARK:
	case G_UNICODE_ENCLOSING_MARK:
	case G_UNICODE_NON_SPACING_MARK:
	case G_UNICODE_DECIMAL_NUMBER:
	case G_UNICODE_LETTER_NUMBER:
	case G_UNICODE_OTHER_NUMBER:
	case G_UNICODE_CONNECT_PUNCTUATION:
		return 1;     /* Enchant 1.3.0 defines word chars like this. */

	case G_UNICODE_DASH_PUNCTUATION:
		if ((n == 1) && (type == G_UNICODE_DASH_PUNCTUATION))
			return 1; /* hyphens only accepted within a word. */
		/* Fallthrough */

	case G_UNICODE_CONTROL:
	case G_UNICODE_FORMAT:
	case G_UNICODE_UNASSIGNED:
	case G_UNICODE_PRIVATE_USE:
	case G_UNICODE_SURROGATE:
	case G_UNICODE_CLOSE_PUNCTUATION:
	case G_UNICODE_FINAL_PUNCTUATION:
	case G_UNICODE_INITIAL_PUNCTUATION:
	case G_UNICODE_OTHER_PUNCTUATION:
	case G_UNICODE_OPEN_PUNCTUATION:
	case G_UNICODE_CURRENCY_SYMBOL:
	case G_UNICODE_MODIFIER_SYMBOL:
	case G_UNICODE_MATH_SYMBOL:
	case G_UNICODE_OTHER_SYMBOL:
	case G_UNICODE_LINE_SEPARATOR:
	case G_UNICODE_PARAGRAPH_SEPARATOR:
	case G_UNICODE_SPACE_SEPARATOR:
	default:
		return 0;
	}
}

void
enchant_broker_set_ordering (EnchantBroker * broker, const char * const tag, const char * const ordering)
{
	g_return_if_fail (broker);
	g_return_if_fail (tag && strlen(tag));
	g_return_if_fail (ordering && strlen(ordering));

	enchant_broker_clear_error (broker);

	char *tag_dupl = enchant_normalize_dictionary_tag (tag);
	char *ordering_dupl = g_strstrip (g_strdup (ordering));

	if (tag_dupl && strlen(tag_dupl) &&
		ordering_dupl && strlen(ordering_dupl))
		{
			/* we will free ordering_dupl && tag_dupl when the hash is destroyed */
			g_hash_table_insert (broker->provider_ordering, (gpointer)tag_dupl,
					     (gpointer)(ordering_dupl));
		}
	else
		{
			g_free (tag_dupl);
			g_free (ordering_dupl);
		}
}

void
enchant_provider_set_error (EnchantProvider * provider, const char * const err)
{
	g_return_if_fail (provider);
	g_return_if_fail (err);
	g_return_if_fail (g_utf8_validate(err, -1, NULL));

	EnchantBroker * broker = provider->owner;
	g_return_if_fail (broker);

	enchant_broker_set_error (broker, err);
}

const char *
enchant_broker_get_error (EnchantBroker * broker)
{
	g_return_val_if_fail (broker, NULL);

	return broker->error;
}

_GL_ATTRIBUTE_MALLOC char *
enchant_get_user_language(void)
{
#if defined(G_OS_WIN32)
	return g_win32_getlocale ();
#else

	const char * locale = g_getenv ("LANG");

#if defined(HAVE_LC_MESSAGES)
	if(!locale)
		locale = setlocale (LC_MESSAGES, NULL);
#endif

	if(!locale)
		locale = setlocale (LC_ALL, NULL);

	if(!locale || strcmp(locale, "C") == 0)
		locale = "en";

	return strdup (locale);
#endif /* !G_OS_WIN32 */
}


char *
enchant_get_prefix_dir(void)
{
	return enchant_relocate (INSTALLPREFIX);
}

void
enchant_set_prefix_dir(const char *new_prefix)
{
#ifdef ENABLE_RELOCATABLE
	set_relocation_prefix (INSTALLPREFIX, new_prefix);
#else
	(void)new_prefix;
#endif
}

const char * _GL_ATTRIBUTE_CONST
enchant_get_version (void) {
	return ENCHANT_VERSION_STRING;
}
