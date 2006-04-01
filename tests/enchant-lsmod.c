/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2003 Dom Lachowicz
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
 * the non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers. If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "enchant.h"
#include "enchant-provider.h"

static void
describe_dict (const char * const lang_tag,
	       const char * const provider_name,
	       const char * const provider_desc,
	       const char * const provider_file,
	       void * user_data)
{
	FILE * out = (FILE *)user_data;
	fprintf (out, "%s (%s)\n", lang_tag, provider_name);
}

static void
enumerate_providers (const char * name,
		     const char * desc,
		     const char * file,
		     void * user_data)
{
	FILE * out = (FILE *)user_data;
	fprintf (out, "%s (%s)\n", name, desc);
}

static void
enumerate_dicts (const char * const lang_tag,
		 const char * const provider_name,
		 const char * const provider_desc,
		 const char * const provider_file,
		 void * user_data)
{
	FILE * out = (FILE *)user_data;
	fprintf (out, "%s (%s)\n", lang_tag, provider_name);
}

int
main (int argc, char **argv)
{
	EnchantBroker *broker;
	EnchantDict *dict;
	char * lang_tag = NULL;
	
	int mode = 0, i;

	for (i = 1; i < argc; i++) {
		if (!strcmp (argv[i], "-lang")) {
			if (i < (argc - 1)) {
				lang_tag = g_strdup (argv[++i]);
			} else {
				lang_tag = enchant_get_user_language();

				if (!lang_tag || !strcmp (lang_tag, "C")) {
					if (lang_tag) /* lang might be "C" */
						g_free (lang_tag);
					lang_tag = g_strdup ("en");
				}
			}
			mode = 1;
		} else if (!strcmp (argv[i], "-h") || !strcmp (argv[i], "-?") || !strcmp(argv[i], "-help")) {
			printf ("%s [-lang [language_tag]] [-list-dicts] [-h] [-v]\n", argv[0]);
			if (lang_tag)
				g_free (lang_tag);
			return 0;
		} else if (!strcmp (argv[i], "-v") || !strcmp (argv[i], "-version")) {
			printf ("%s %s\n", argv[0], VERSION);
			if (lang_tag)
				g_free (lang_tag);
			return 0;
		} else if (!strcmp (argv[i], "-list-dicts")) {
			mode = 2;
		}
	}
	
	broker = enchant_broker_init ();
	
	if (mode == 0) {
		enchant_broker_describe (broker, enumerate_providers, stdout);
	} else if (mode == 1) {

		if (!lang_tag) {
			printf ("Error: language tag not specified and environment variable $LANG not set.\n");
			enchant_broker_free (broker);
			return 1;
		}

		dict = enchant_broker_request_dict (broker, lang_tag);
		
		if (!dict) {
			printf ("No dictionary available for '%s'.\n", lang_tag);

			if (lang_tag)
				g_free (lang_tag);

			enchant_broker_free (broker);
			return 1;
		} else {
			enchant_dict_describe (dict, describe_dict, stdout);
			enchant_broker_free_dict (broker, dict);
		}
	} else if (mode == 2) {
		enchant_broker_list_dicts (broker, enumerate_dicts, stdout);
	}

	if (lang_tag)
		g_free (lang_tag);
	
	enchant_broker_free (broker);
	
	return 0;
}
