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

#ifdef _WIN32
#include <windows.h>
#endif

#include "enchant.h"
#include "enchant-provider.h"
#include "pwl.h"
#include "unused-parameter.h"
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
	EnchantSession * session;
	EnchantPWL *personal = NULL;
	EnchantPWL *exclude = NULL;

	if (pwl)
		personal = enchant_pwl_init_with_file (pwl);

	if (personal == NULL) {
		if (fail_if_no_pwl)
			return NULL;
		else
			personal = enchant_pwl_init ();
	}

	if (excl)
		exclude = enchant_pwl_init_with_file (excl);
	if (exclude == NULL)
		exclude = enchant_pwl_init ();

	session = g_new0 (EnchantSession, 1);
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
_enchant_session_new (EnchantProvider *provider, const char * const user_config_dir,
		      const char * const lang, gboolean fail_if_no_pwl)
{
	char *filename, *dic, *excl;
	EnchantSession * session;

	if (!user_config_dir || !lang)
		return NULL;

	filename = g_strdup_printf ("%s.dic", lang);
	dic = g_build_filename (user_config_dir, filename, NULL);
	g_free (filename);

	filename = g_strdup_printf ("%s.exc", lang);
	excl = g_build_filename (user_config_dir, filename, NULL);
	g_free (filename);

	session = enchant_session_new_with_pwl (provider, dic, excl, lang, fail_if_no_pwl);

	g_free (dic);
	g_free (excl);

	return session;
}

static EnchantSession *
enchant_session_new (EnchantProvider *provider, const char * const lang)
{
	char *user_config_dir = enchant_get_user_config_dir ();

	EnchantSession * session = NULL;
	session = _enchant_session_new (provider, user_config_dir, lang, TRUE);

	if (session == NULL && user_config_dir != NULL)
		{
			enchant_ensure_dir_exists (user_config_dir);
			session = _enchant_session_new (provider, user_config_dir, lang, FALSE);
		}

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

static void
enchant_session_add_personal (EnchantSession * session, const char * const word, size_t len)
{
	enchant_pwl_add(session->personal, word, len);
}

static void
enchant_session_remove_personal (EnchantSession * session, const char * const word, size_t len)
{
	enchant_pwl_remove(session->personal, word, len);
}

static void
enchant_session_add_exclude (EnchantSession * session, const char * const word, size_t len)
{
	enchant_pwl_add(session->exclude, word, len);
}

static void
enchant_session_remove_exclude (EnchantSession * session, const char * const word, size_t len)
{
	enchant_pwl_remove(session->exclude, word, len);
}

/* a word is excluded if it is in the exclude dictionary or in the session exclude list
 *  AND the word has not been added to the session include list
 */
static gboolean
enchant_session_exclude (EnchantSession * session, const char * const word, size_t len)
{
	gboolean result = FALSE;

	char * utf = g_strndup (word, len);

	if (!g_hash_table_lookup (session->session_include, utf) &&
			(g_hash_table_lookup (session->session_exclude, utf)||
			 enchant_pwl_check (session->exclude, word, len) == 0 ))
			result = TRUE;
	g_free (utf);

	return result;
}

static gboolean
enchant_session_contains (EnchantSession * session, const char * const word, size_t len)
{
	gboolean result = FALSE;

	char * utf = g_strndup (word, len);

	if (g_hash_table_lookup (session->session_include, utf) ||
		(enchant_pwl_check (session->personal, word, len) == 0 &&
		 (!enchant_pwl_check (session->exclude, word, len)) == 0))
		result = TRUE;

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
enchant_provider_free_string_list (EnchantProvider * provider, char ** string_list)
{
	if (provider && provider->free_string_list)
		(*provider->free_string_list) (provider, string_list);
}

ENCHANT_MODULE_EXPORT(void)
enchant_dict_set_error (EnchantDict * dict, const char * const err)
{
	EnchantSession * session;

	g_return_if_fail (dict);
	g_return_if_fail (err);
	g_return_if_fail (g_utf8_validate(err, -1, NULL));

	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;

	enchant_session_clear_error (session);
	session->error = strdup (err);
}

ENCHANT_MODULE_EXPORT(char *)
enchant_dict_get_error (EnchantDict * dict)
{
	EnchantSession * session;

	g_return_val_if_fail (dict, NULL);

	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	return session->error;
}

ENCHANT_MODULE_EXPORT (int)
enchant_dict_check (EnchantDict * dict, const char *const word, ssize_t len)
{
	EnchantSession * session;

	g_return_val_if_fail (dict, -1);
	g_return_val_if_fail (word, -1);

	if (len < 0)
		len = strlen (word);

	g_return_val_if_fail (len, -1);
	g_return_val_if_fail (g_utf8_validate(word, len, NULL),-1);

	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
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

static char **
enchant_dict_get_good_suggestions(EnchantDict * dict,
								char ** suggs,
								size_t n_suggs,
								size_t* out_n_filtered_suggs)
{
	EnchantSession * session;
	size_t i, n_filtered_suggs;
	char ** filtered_suggs;

	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;

	filtered_suggs = g_new0 (char *, n_suggs + 1);
	n_filtered_suggs = 0;

	for(i = 0; i < n_suggs; i++)
		{
			size_t sugg_len = strlen(suggs[i]);

			if (sugg_len == 0) continue;

			if(g_utf8_validate(suggs[i], sugg_len, NULL) &&
			   !enchant_session_exclude(session, suggs[i], sugg_len) )
				{
					filtered_suggs[n_filtered_suggs] = strdup (suggs[i]);
					++n_filtered_suggs;
				}
		}

	if(out_n_filtered_suggs)
		*out_n_filtered_suggs = n_filtered_suggs;

	return filtered_suggs;
}

ENCHANT_MODULE_EXPORT (char **)
enchant_dict_suggest (EnchantDict * dict, const char *const word,
			  ssize_t len, size_t * out_n_suggs)
{
	EnchantSession * session;
	size_t n_suggs = 0, n_dict_suggs = 0, n_pwl_suggs = 0, n_suggsT = 0;
	char **suggs, **dict_suggs = NULL, **pwl_suggs = NULL, **suggsT;

	g_return_val_if_fail (dict, NULL);
	g_return_val_if_fail (word, NULL);

	if (len < 0)
		len = strlen (word);

	g_return_val_if_fail (len, NULL);
	g_return_val_if_fail (g_utf8_validate(word, len, NULL), NULL);

	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);
	/* Check for suggestions from provider dictionary */
	if (dict->suggest)
		{
			dict_suggs = (*dict->suggest) (dict, word, len,
							&n_dict_suggs);
			if(dict_suggs)
				{
					suggsT = enchant_dict_get_good_suggestions(dict, dict_suggs, n_dict_suggs, &n_suggsT);
					enchant_provider_free_string_list (session->provider, dict_suggs);
					dict_suggs = suggsT;
					n_dict_suggs = n_suggsT;
				}
		}

	/* Check for suggestions from personal dictionary */
	if(session->personal)
		{
			pwl_suggs = enchant_pwl_suggest(session->personal, word, len, dict_suggs, &n_pwl_suggs);
			if(pwl_suggs)
				{
					suggsT = enchant_dict_get_good_suggestions(dict, pwl_suggs, n_pwl_suggs, &n_suggsT);
					enchant_pwl_free_string_list (session->personal, pwl_suggs);
					pwl_suggs = suggsT;
					n_pwl_suggs = n_suggsT;
				}
		}
	/* Clone suggestions if there are any */
	n_suggs = n_pwl_suggs + n_dict_suggs;
	if (n_suggs > 0)
		{
			suggs = g_new0 (char *, n_suggs + 1);

			/* Copy over suggestions from dict, if no dupes */
			n_suggs = enchant_dict_merge_suggestions(suggs, 0,
								 dict_suggs, n_dict_suggs);

			/* Copy over suggestions from pwl, if no dupes */
			n_suggs = enchant_dict_merge_suggestions(suggs, n_suggs,
								 pwl_suggs, n_pwl_suggs);
			if(n_suggs == 0)
			{
				g_free(suggs);
				suggs = NULL;
			}
		}
	else
		{
			suggs = NULL;
		}

	g_strfreev(dict_suggs);
	g_strfreev(pwl_suggs);

	if (out_n_suggs)
		*out_n_suggs = n_suggs;

	return suggs;
}

ENCHANT_MODULE_EXPORT (void)
enchant_dict_add (EnchantDict * dict, const char *const word,
			 ssize_t len)
{
	EnchantSession * session;

	g_return_if_fail (dict);
	g_return_if_fail (word);

	if (len < 0)
		len = strlen (word);

	g_return_if_fail (len);
	g_return_if_fail (g_utf8_validate(word, len, NULL));

	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);
	enchant_session_add_personal (session, word, len);
	enchant_session_remove_exclude (session, word, len);

	if (dict->add_to_personal)
		(*dict->add_to_personal) (dict, word, len);
}

ENCHANT_MODULE_EXPORT (void)
enchant_dict_add_to_session (EnchantDict * dict, const char *const word,
				 ssize_t len)
{
	EnchantSession * session;

	g_return_if_fail (dict);
	g_return_if_fail (word);

	if (len < 0)
		len = strlen (word);

	g_return_if_fail (len);
	g_return_if_fail (g_utf8_validate(word, len, NULL));

	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	enchant_session_add (session, word, len);
	if (dict->add_to_session)
		(*dict->add_to_session) (dict, word, len);
}

ENCHANT_MODULE_EXPORT (int)
enchant_dict_is_added (EnchantDict * dict, const char *const word,
				ssize_t len)
{
	EnchantSession * session;

	g_return_val_if_fail (dict, 0);
	g_return_val_if_fail (word, 0);

	if (len < 0)
		len = strlen (word);

	g_return_val_if_fail (len, 0);
	g_return_val_if_fail (g_utf8_validate(word, len, NULL), 0);

	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	return enchant_session_contains (session, word, len);
}

ENCHANT_MODULE_EXPORT (void)
enchant_dict_remove (EnchantDict * dict, const char *const word,
			 ssize_t len)
{
	EnchantSession * session;

	g_return_if_fail (dict);
	g_return_if_fail (word);

	if (len < 0)
		len = strlen (word);

	g_return_if_fail (len);
	g_return_if_fail (g_utf8_validate(word, len, NULL));

	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	enchant_session_remove_personal (session, word, len);
	enchant_session_add_exclude(session, word, len);

	if (dict->add_to_exclude)
		(*dict->add_to_exclude) (dict, word, len);
}

ENCHANT_MODULE_EXPORT (void)
enchant_dict_remove_from_session (EnchantDict * dict, const char *const word,
			 ssize_t len)
{
	EnchantSession * session;

	g_return_if_fail (dict);
	g_return_if_fail (word);

	if (len < 0)
		len = strlen (word);

	g_return_if_fail (len);
	g_return_if_fail (g_utf8_validate(word, len, NULL));

	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	enchant_session_remove (session, word, len);
}

ENCHANT_MODULE_EXPORT (int)
enchant_dict_is_removed (EnchantDict * dict, const char *const word,
				ssize_t len)
{
	EnchantSession * session;

	g_return_val_if_fail (dict, 0);
	g_return_val_if_fail (word, 0);

	if (len < 0)
		len = strlen (word);

	g_return_val_if_fail (len, 0);
	g_return_val_if_fail (g_utf8_validate(word, len, NULL), 0);

	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	return enchant_session_exclude (session, word, len);
}

ENCHANT_MODULE_EXPORT (void)
enchant_dict_store_replacement (EnchantDict * dict,
				const char *const mis, ssize_t mis_len,
				const char *const cor, ssize_t cor_len)
{
	EnchantSession * session;

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

	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);

	/* if it's not implemented, it's not worth emulating */
	if (dict->store_replacement)
		(*dict->store_replacement) (dict, mis, mis_len, cor, cor_len);
}

ENCHANT_MODULE_EXPORT (void)
enchant_dict_free_string_list (EnchantDict * dict, char **string_list)
{
	EnchantSession * session;

	g_return_if_fail (dict);
	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);
	g_strfreev(string_list);
}

ENCHANT_MODULE_EXPORT (void)
enchant_dict_describe (EnchantDict * dict,
			   EnchantDictDescribeFn fn,
			   void * user_data)
{
	EnchantSession * session;
	EnchantProvider * provider;
	GModule *module;

	const char * tag, * name, * desc, * file;

	g_return_if_fail (dict);
	g_return_if_fail (fn);

	session = ((EnchantDictPrivateData*)dict->enchant_private_data)->session;
	enchant_session_clear_error (session);
	provider = session->provider;

	if (provider)
		{
			module = (GModule *) provider->enchant_private_data;
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

	tag = session->language_tag;
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
	if(provider == NULL)
		{
			g_warning ("EnchantProvider cannot be NULL\n");
			return 0;
		}

	if(provider->identify == NULL)
		{
			g_warning ("EnchantProvider's identify method cannot be NULL\n");
			return 0;
		}
	else if(!g_utf8_validate((*provider->identify)(provider), -1, NULL))
		{
			g_warning ("EnchantProvider's identify method does not return valid utf8.\n");
			return 0;
		}

	if(provider->describe == NULL)
		{
			g_warning ("EnchantProvider's describe method cannot be NULL\n");
			return 0;
		}
	else if(!g_utf8_validate((*provider->describe)(provider), -1, NULL))
		{
			g_warning ("EnchantProvider's describe method does not return valid utf8.\n");
			return 0;
		}

	return 1;
}

static void
enchant_load_providers_in_dir (EnchantBroker * broker, const char *dir_name)
{
	GModule *module = NULL;
	GDir *dir;
	const char *dir_entry;
	size_t entry_len, g_module_suffix_len;

	char * filename;

	EnchantProvider *provider;
	EnchantProviderInitFunc init_func;
	EnchantPreConfigureFunc conf_func;

	dir = g_dir_open (dir_name, 0, NULL);
	if (!dir)
		return;

	g_module_suffix_len = strlen (G_MODULE_SUFFIX);

	while ((dir_entry = g_dir_read_name (dir)) != NULL)
		{
			provider = 0;

			entry_len = strlen (dir_entry);
			if ((entry_len > g_module_suffix_len) &&
				!strcmp(dir_entry+(entry_len-g_module_suffix_len), G_MODULE_SUFFIX))
				{
#ifdef _WIN32
					/* Suppress error popups for failing to load plugins */
					UINT old_error_mode = SetErrorMode(SEM_FAILCRITICALERRORS);
#endif
					filename = g_build_filename (dir_name, dir_entry, NULL);

					module = g_module_open (filename, (GModuleFlags) 0);
					if (module)
						{
							if (g_module_symbol
								(module, "init_enchant_provider", (gpointer *) (&init_func))
								&& init_func)
								{
									provider = init_func ();
									if (!enchant_provider_is_valid(provider))
										{
											g_warning ("Error loading plugin: %s's init_enchant_provider returned invalid provider.\n", dir_entry);
											if(provider)
												{
													if(provider->dispose)
														provider->dispose(provider);

													provider = NULL;
												}
											g_module_close (module);
										}
								}
							else
								{
									g_module_close (module);
								}
						}
					else
						{
							g_warning ("Error loading plugin: %s\n", g_module_error());
						}

					g_free (filename);
#ifdef _WIN32
					/* Restore the original error mode */
					SetErrorMode(old_error_mode);
#endif
				}
			if (provider)
				{
					/* optional entry point to allow modules to look for associated files
					 */
					if (g_module_symbol
						(module, "configure_enchant_provider", (gpointer *) (&conf_func))
						&& conf_func)
						{
							conf_func (provider, dir_name);
							if (!enchant_provider_is_valid(provider))
								{
									g_warning ("Error loading plugin: %s's configure_enchant_provider modified provider and it is now invalid.\n", dir_entry);
									if(provider->dispose)
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
	char *module_dir = enchant_relocate (PKGLIBDIR);
	enchant_load_providers_in_dir (broker, module_dir);
	free (module_dir);
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

static void
enchant_load_provider_ordering (EnchantBroker * broker)
{
	GSList *conf_dirs, *iter;

	broker->provider_ordering = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

	conf_dirs = enchant_get_conf_dirs ();
	for (iter = conf_dirs; iter; iter = iter->next)
		{
			char *ordering_file;
			ordering_file = g_build_filename (iter->data, "enchant.ordering", NULL);
			enchant_load_ordering_from_file (broker, ordering_file);
			g_free (ordering_file);
		}

	g_slist_free_full (conf_dirs, g_free);
}

static GSList *
enchant_get_ordered_providers (EnchantBroker * broker,
				   const char * const tag)
{
	EnchantProvider *provider;
	GSList * list = NULL, * iter = NULL;

	char * ordering = NULL, ** tokens, *token;
	size_t i;

	ordering = (char *)g_hash_table_lookup (broker->provider_ordering, (gpointer)tag);
	if (!ordering)
		ordering = (char *)g_hash_table_lookup (broker->provider_ordering, (gpointer)"*");

	if (!ordering)
		{
			/* return an unordered copy of the list */
			for (iter = broker->provider_list; iter != NULL; iter = g_slist_next (iter))
					list = g_slist_append (list, iter->data);
			return list;
		}

	tokens = g_strsplit (ordering, ",", 0);
	if (tokens)
		{
			for (i = 0; tokens[i]; i++)
				{
					token = g_strstrip(tokens[i]);

					for (iter = broker->provider_list; iter != NULL; iter = g_slist_next (iter))
						{
							provider = (EnchantProvider*)iter->data;

							if (provider && !strcmp (token, (*provider->identify)(provider)))
								list = g_slist_append (list, (gpointer)provider);
						}
				}

			g_strfreev (tokens);
		}

	/* providers not in the list need to be appended at the end */
	for (iter = broker->provider_list; iter != NULL; iter = g_slist_next (iter))
		{
			if (!g_slist_find (list, iter->data))
				list = g_slist_append (list, iter->data);
		}

	return list;
}

static void
enchant_dict_destroyed (gpointer data)
{
	EnchantDict *dict;
	EnchantProvider *owner;
	EnchantSession *session;
	EnchantDictPrivateData *enchant_dict_private_data;

	g_return_if_fail (data);

	dict = (EnchantDict *) data;
	enchant_dict_private_data = (EnchantDictPrivateData*)dict->enchant_private_data;
	session = enchant_dict_private_data->session;
	owner = session->provider;

	if (owner && owner->dispose_dict)
		(*owner->dispose_dict) (owner, dict);
	else if(session->is_pwl)
		g_free (dict);

	g_free(enchant_dict_private_data);

	enchant_session_destroy (session);
}

static void
enchant_provider_free (gpointer data)
{
	EnchantProvider *provider;
	GModule *module;

	g_return_if_fail (data);

	provider = (EnchantProvider *) data;
	module = (GModule *) provider->enchant_private_data;

	if (provider->dispose)
		(*provider->dispose) (provider);

	/* close module only after invoking dispose */
	g_module_close (module);
}

ENCHANT_MODULE_EXPORT (EnchantBroker *)
enchant_broker_init (void)
{
	EnchantBroker *broker = NULL;

	g_return_val_if_fail (g_module_supported (), NULL);

	broker = g_new0 (EnchantBroker, 1);

	broker->dict_map = g_hash_table_new_full (g_str_hash, g_str_equal,
						  g_free, enchant_dict_destroyed);
	enchant_load_providers (broker);
	enchant_load_provider_ordering (broker);

	return broker;
}

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
	g_hash_table_destroy (broker->provider_ordering);

	g_slist_free_full (broker->provider_list, enchant_provider_free);

	enchant_broker_clear_error (broker);

	g_free (broker);
}

ENCHANT_MODULE_EXPORT (EnchantDict *)
enchant_broker_request_pwl_dict (EnchantBroker * broker, const char *const pwl)
{
	EnchantSession *session;
	EnchantDictPrivateData *enchant_dict_private_data;
	EnchantDict *dict = NULL;

	g_return_val_if_fail (broker, NULL);
	g_return_val_if_fail (pwl && strlen(pwl), NULL);

	enchant_broker_clear_error (broker);

	dict = (EnchantDict*)g_hash_table_lookup (broker->dict_map, (gpointer) pwl);
	if (dict) {
		((EnchantDictPrivateData*)dict->enchant_private_data)->reference_count++;
		return dict;
	}

	/* since the broker pwl file is a read/write file (there is no readonly dictionary associated)
	 * there is no need for complementary exclude file to add a word to. The word just needs to be
	 * removed from the broker pwl file
	 */
	session = enchant_session_new_with_pwl (NULL, pwl, NULL, "Personal Wordlist", TRUE);
	if (!session)
		{
			broker->error = g_strdup_printf ("Couldn't open personal wordlist '%s'", pwl);
			return NULL;
		}

	session->is_pwl = 1;

	dict = g_new0 (EnchantDict, 1);
	enchant_dict_private_data = g_new0 (EnchantDictPrivateData, 1);
	enchant_dict_private_data->reference_count = 1;
	enchant_dict_private_data->session = session;
	dict->enchant_private_data = (void *)enchant_dict_private_data;

	g_hash_table_insert (broker->dict_map, (gpointer)strdup (pwl), dict);

	return dict;
}

static EnchantDict *
_enchant_broker_request_dict (EnchantBroker * broker, const char *const tag)
{
	EnchantDict * dict;
	GSList * list;
	GSList * listIter;

	dict = (EnchantDict*)g_hash_table_lookup (broker->dict_map, (gpointer) tag);
	if (dict) {
		((EnchantDictPrivateData*)dict->enchant_private_data)->reference_count++;
		return dict;
	}

	list = enchant_get_ordered_providers (broker, tag);
	for (listIter = list; listIter != NULL; listIter = g_slist_next (listIter))
		{
			EnchantProvider * provider;

			provider = (EnchantProvider *) listIter->data;

			if (provider->request_dict)
				{
					dict = (*provider->request_dict) (provider, tag);

					if (dict)
						{
							EnchantSession *session;
							EnchantDictPrivateData *enchant_dict_private_data;

							session = enchant_session_new (provider, tag);
							enchant_dict_private_data = g_new0 (EnchantDictPrivateData, 1);
							enchant_dict_private_data->reference_count = 1;
							enchant_dict_private_data->session = session;
							dict->enchant_private_data = (void *)enchant_dict_private_data;
							g_hash_table_insert (broker->dict_map, (gpointer)strdup (tag), dict);
							break;
						}
				}
		}

	g_slist_free (list);

	return dict;
}

ENCHANT_MODULE_EXPORT (EnchantDict *)
enchant_broker_request_dict (EnchantBroker * broker, const char *const tag)
{
	EnchantDict *dict = NULL;
	char * normalized_tag;

	g_return_val_if_fail (broker, NULL);
	g_return_val_if_fail (tag && strlen(tag), NULL);

	enchant_broker_clear_error (broker);

	normalized_tag = enchant_normalize_dictionary_tag (tag);
	if(!enchant_is_valid_dictionary_tag(normalized_tag))
		{
			enchant_broker_set_error (broker, "invalid tag character found");
		}
	else if ((dict = _enchant_broker_request_dict (broker, normalized_tag)) == NULL)
		{
			char * iso_639_only_tag;

			iso_639_only_tag = enchant_iso_639_from_tag (normalized_tag);

			dict = _enchant_broker_request_dict (broker, iso_639_only_tag);

			free (iso_639_only_tag);
		}

	free (normalized_tag);

	return dict;
}

ENCHANT_MODULE_EXPORT (void)
enchant_broker_describe (EnchantBroker * broker,
			 EnchantBrokerDescribeFn fn,
			 void * user_data)
{
	GSList *list;
	EnchantProvider *provider;
	GModule *module;

	const char * name, * desc, * file;

	g_return_if_fail (broker);
	g_return_if_fail (fn);

	enchant_broker_clear_error (broker);

	for (list = broker->provider_list; list != NULL; list = g_slist_next (list))
		{
			provider = (EnchantProvider *) list->data;
			module = (GModule *) provider->enchant_private_data;

			name = (*provider->identify) (provider);
			desc = (*provider->describe) (provider);
			file = g_module_name (module);

			(*fn) (name, desc, file, user_data);
		}
}

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

	for (list = broker->provider_list; list != NULL; list = g_slist_next (list))
		{
			EnchantProvider *provider;

			provider = (EnchantProvider *) list->data;

			if (provider->list_dicts)
				{
					size_t n_dicts, i;
					char ** dicts;

					dicts = (*provider->list_dicts) (provider, &n_dicts);

					for (i = 0; i < n_dicts; i++)
						{
							const char * tag;

							tag = dicts[i];
							if (enchant_is_valid_dictionary_tag (tag)) {
								gpointer ptr;
								GSList *providers;
								gint this_priority;

								providers = enchant_get_ordered_providers (broker, tag);
								this_priority = g_slist_index (providers, provider);
								if (this_priority != -1) {
									gint min_priority;

									min_priority = this_priority + 1;
									ptr = g_hash_table_lookup (tags, tag);
									if (ptr != NULL)
										min_priority = g_slist_index (providers, ptr);
									if (this_priority < min_priority)
										g_hash_table_insert (tags, strdup (tag), provider);
								}
								g_slist_free (providers);
							}
						}

					enchant_provider_free_string_list (provider, dicts);
				}
		}

	g_hash_table_iter_init (&iter, tags);
	while (g_hash_table_iter_next (&iter, &key, &value))
		{
			const char * tag, * name, * desc, * file;
			EnchantProvider * provider;
			GModule *module;

			tag = (const char *) key;
			provider = (EnchantProvider *) value;
			module = (GModule *) provider->enchant_private_data;
			name = (*provider->identify) (provider);
			desc = (*provider->describe) (provider);
			file = g_module_name (module);
			(*fn) (tag, name, desc, file, user_data);
		}

	g_hash_table_destroy (tags);
}

ENCHANT_MODULE_EXPORT (void)
enchant_broker_free_dict (EnchantBroker * broker, EnchantDict * dict)
{
	EnchantSession * session;
	EnchantDictPrivateData * dict_private_data;

	g_return_if_fail (broker);
	g_return_if_fail (dict);

	enchant_broker_clear_error (broker);

	dict_private_data = (EnchantDictPrivateData*)dict->enchant_private_data;
	dict_private_data->reference_count--;
	if(dict_private_data->reference_count == 0)
		{
			session = dict_private_data->session;

			if (session->provider)
				g_hash_table_remove (broker->dict_map, session->language_tag);
			else
				g_hash_table_remove (broker->dict_map, session->personal_filename);
		}
}

static int
enchant_provider_dictionary_exists (EnchantProvider * provider,
					 const char * const tag)
{
	int exists = 0;

	if (provider->dictionary_exists)
		{
			exists = (*provider->dictionary_exists) (provider, tag);
		}
	else if (provider->list_dicts)
		{
			size_t n_dicts, i;
			char ** dicts;

			dicts = (*provider->list_dicts) (provider, &n_dicts);

			for (i = 0; (i < n_dicts) && !exists; i++)
				{
					if (!strcmp(dicts[i], tag))
						exists = 1;
				}

			enchant_provider_free_string_list (provider, dicts);
		}
	else if (provider->request_dict)
		{
			EnchantDict *dict;

			dict = (*provider->request_dict) (provider, tag);
			if (dict)
				{
					if (provider->dispose_dict)
						(*provider->dispose_dict) (provider, dict);
					exists = 1;
				}
		}

	return exists;
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

	for (list = broker->provider_list; list != NULL; list = g_slist_next (list))
		{
			EnchantProvider * provider;

			provider = (EnchantProvider *) list->data;

			if (enchant_provider_dictionary_exists (provider, tag))
				{
					return 1;
				}
		}

	return 0;
}

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

ENCHANT_MODULE_EXPORT(void)
enchant_provider_set_error (EnchantProvider * provider, const char * const err)
{
	EnchantBroker * broker;

	g_return_if_fail (provider);
	g_return_if_fail (err);
	g_return_if_fail (g_utf8_validate(err, -1, NULL));

	broker = provider->owner;
	g_return_if_fail (broker);

	enchant_broker_set_error (broker, err);
}

ENCHANT_MODULE_EXPORT(char *)
enchant_broker_get_error (EnchantBroker * broker)
{
	g_return_val_if_fail (broker, NULL);

	return broker->error;
}

ENCHANT_MODULE_EXPORT(char *)
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


ENCHANT_MODULE_EXPORT (char *)
enchant_get_prefix_dir(void)
{
	return enchant_relocate (INSTALLPREFIX);
}

ENCHANT_MODULE_EXPORT (void)
enchant_set_prefix_dir(const char *new_prefix)
{
	set_relocation_prefix (INSTALLPREFIX, new_prefix);
}

ENCHANT_MODULE_EXPORT(const char *) _GL_ATTRIBUTE_CONST
enchant_get_version (void) {
	return ENCHANT_VERSION_STRING;
}
