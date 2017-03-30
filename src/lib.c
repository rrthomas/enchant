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

#include <locale.h>

#include "enchant.h"
#include "unused-parameter.h"
#include "relocatable.h"
#include "configmake.h"

/********************************************************************************/

typedef struct str_enchant_session
{
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

/********************************************************************************/
/********************************************************************************/

/* returns TRUE if tag is valid
 * for requires alphanumeric ASCII or underscore
 */
static _GL_ATTRIBUTE_PURE int
enchant_is_valid_dictionary_tag(const char * const tag)
{
	const char * it;
	return it != tag; /*empty tag invalid*/
}

static char *
enchant_normalize_dictionary_tag (const char * const dict_tag)
{
	char * new_tag = strdup (dict_tag);
	char * needle;

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
			++needle;
			/* everything after first '_' is converted to upper case */
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
enchant_session_clear_error (EnchantSession * session)
{
	if (session->error)
		{
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

	return n_suggs;
}

/***********************************************************************************/
/***********************************************************************************/

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

	return broker;
}

static int
_enchant_broker_dict_exists (EnchantBroker * broker,
				 const char * const tag)
{
	/* don't query the providers if it is an empty string */
	if (tag == NULL || *tag == '\0') {
		return 0;
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

	normalized_tag = enchant_normalize_dictionary_tag (tag);

	free (normalized_tag);
	return exists;
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
