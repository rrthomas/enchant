/* enchant
 * Copyright (C) 2003 Dom Lachowicz
 * Copyright (C) 2017 Reuben Thomas <rrt@sc3d.org>
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
 * the non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers. If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#include "config.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unused-parameter.h"

#include "enchant.h"
#include "enchant-provider.h"

static void
describe_dict (const char * const lang_tag,
	       const char * const provider_name,
	       const char * const provider_desc _GL_UNUSED_PARAMETER,
	       const char * const provider_file _GL_UNUSED_PARAMETER,
	       void * user_data _GL_UNUSED_PARAMETER)
{
	printf ("%s (%s)\n", lang_tag, provider_name);
}

static void
describe_word_chars (const char * const lang_tag _GL_UNUSED_PARAMETER,
		     const char * const provider_name _GL_UNUSED_PARAMETER,
		     const char * const provider_desc _GL_UNUSED_PARAMETER,
		     const char * const provider_file _GL_UNUSED_PARAMETER,
		     void * user_data)
{
	EnchantDict *dict = (EnchantDict *)user_data;
	const char *word_chars = "";
	if (dict)
		word_chars = enchant_dict_get_extra_word_characters(dict);
	printf ("%s\n", word_chars ? word_chars : "");
}

static void
describe_provider (const char * name,
		   const char * desc,
		   const char * file _GL_UNUSED_PARAMETER,
		   void * user_data _GL_UNUSED_PARAMETER)
{
	printf ("%s (%s)\n", name, desc);
}

static void
usage (const char *progname)
{
	fprintf (stderr, "%s [[-lang|-word-chars] [language_tag]|-list-dicts|-help|-version]\n", progname);
}

int
main (int argc, char **argv)
{
	EnchantBroker *broker = enchant_broker_init ();
	char * lang_tag = NULL;
	int retcode = 0;

	if (argc > 1) {
		if (!strcmp (argv[1], "-lang") || !strcmp(argv[1], "-word-chars")) {
			if (argc > 2) {
				lang_tag = strdup (argv[2]);
			} else {
				lang_tag = enchant_get_user_language();

				if (!lang_tag || !strcmp (lang_tag, "C")) {
					free(lang_tag);
					lang_tag = strdup ("en");
				}
			}
			if (!lang_tag) {
				fprintf (stderr, "Error: language tag not specified and environment variable $LANG not set\n");
				retcode = 1;
			} else {
				EnchantDict *dict = enchant_broker_request_dict (broker, lang_tag);
				if (!dict) {
					fprintf (stderr, "No dictionary available for '%s'", lang_tag);
					const char *errmsg = enchant_broker_get_error (broker);
					if (errmsg != NULL)
						fprintf (stderr, ": %s", errmsg);
					putc('\n', stderr);
					retcode = 1;
				} else {
					enchant_dict_describe (dict,
							       !strcmp (argv[1], "-lang") ? describe_dict : describe_word_chars,
							       dict);
					enchant_broker_free_dict (broker, dict);
				}
			}
		} else if (!strcmp (argv[1], "-h") || !strcmp(argv[1], "-help")) {
			usage (argv[0]);
		} else if (!strcmp (argv[1], "-v") || !strcmp (argv[1], "-version")) {
			fprintf (stderr, "%s %s\n", argv[0], PACKAGE_VERSION);
		} else if (!strcmp (argv[1], "-list-dicts")) {
			enchant_broker_list_dicts (broker, describe_dict, NULL);
		} else {
			fprintf (stderr, "Invalid argument %s\n", argv[1]);
			usage (argv[0]);
			retcode = 1;
		}
	} else
		enchant_broker_describe (broker, describe_provider, NULL);

	free (lang_tag);
	enchant_broker_free (broker);
	return retcode;
}
