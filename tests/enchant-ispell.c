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
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "enchant.h"

typedef enum 
	{
		MODE_NONE,
		MODE_VERSION,
		MODE_A,
		MODE_L,
		MODE_FILE
	} IspellMode_t;

static void 
print_version (FILE * to)
{
	fprintf (to, "@(#) International Ispell Version 3.1.20 (but really Enchant %s)\n", VERSION);
}

static void
print_help (FILE * to, const char * prog)
{
	fprintf (to, "Usage: %s [options] -a|-l|-v[v]|<file>\n", prog);
}

static char *
consume_next_word (FILE * in, size_t * start_pos)
{
	/* TODO: the word parsing routine - use glib's unicode functions */
	*start_pos = 0;
	return NULL;
}

static int
parse_file (FILE * in, FILE * out, IspellMode_t mode)
{
	EnchantBroker * broker;
	EnchantDict * dict;
	
	const gchar * lang;
	
	char * word = NULL;
	size_t start_pos = 0, word_count = 0;
	
	if (mode == MODE_A)
		print_version (out);
	
	lang = g_getenv ("LANG");
	if (!lang || !strcmp (lang, "C"))
		lang = "en";
	
	broker = enchant_broker_init ();
	dict = enchant_broker_request_dict (broker, lang);
	if (!dict) {
		fprintf (stderr, "Couldn't create a dictionary for %s\n", lang);
		enchant_broker_term (broker);
	}
	
	while ((word = consume_next_word (in, &start_pos)) != NULL) {
		word_count++;
		if (mode == MODE_A) {
			if (enchant_dict_check (dict, word, strlen (word)) == 0)
				fprintf (out, "*\n");
			else {
				size_t n_suggs;
				char ** suggs;
				
				suggs = enchant_dict_suggest (dict, word, 
							      strlen (word), &n_suggs);
				if (!n_suggs || !suggs)
					fprintf (out, "# %s %ld\n", word, start_pos);
				else {
					size_t i = 0;
					
					fprintf (out, "& %s %ld %ld:", word, n_suggs, start_pos);
					
					for (i = 0; i < n_suggs; i++) {
						if (i != (n_suggs - 1))
							fprintf (out, " %s,", word);
						else
							fprintf (out, " %s\n", word);
					}
				}
			}
		} 
		else if (mode == MODE_L) {
			if (enchant_dict_check (dict, word, strlen (word)) != 0)
				fprintf (out, "%s\n", word);
		}
		
		g_free (word);
	}
	
	enchant_broker_release_dict (broker, dict);
	enchant_broker_term (broker);

	return 0;
}

int main (int argc, char ** argv)
{
	IspellMode_t mode = MODE_NONE;
	
	char * arg, * file = NULL;
	int i, rval = 0;
	
	FILE * fp = stdin;
	
	for (i = 1; i < argc; i++) {
		arg = argv[i];
		if (arg[0] == '-') {
			if (strlen (arg) == 2) {
				if (arg[1] == 'a')
					mode = MODE_A;
				else if (arg[1] == 'l')
					mode = MODE_L;
				else if (arg[1] == 'v')
					mode = MODE_VERSION;
			} 
			else if (strlen (arg) > 2) {
				fprintf (stderr, "-%c does not take any parameters.\n", arg[1]);
				exit(1);
			} 
			else
				file = arg;
		} 
		else
			file = arg;
	}
	
	if (mode == MODE_VERSION) {
		print_version (stdout);
	} 
	else if (mode == MODE_NONE && !file) {
		print_help (stdout, argv[0]);
	}
	else {
		if (file) {
			fp = fopen (file, "r");
			if (!fp) {
				fprintf (stderr, "Couldn't open '%s' to spellcheck\n", file);
				exit (1);
			}
		}
		
		rval = parse_file (fp, stdout, mode);
		
		if (file)
			fclose (fp);
	}
	
	return rval;
}
