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

/**
 * This is a rough approximation of an "ispell compatibility mode"
 * for Enchant.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "enchant.h"

/* word has to be bigger than this to be checked */
#define MIN_WORD_LENGTH 1

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
	fprintf (to, "Usage: %s [options] -a|-l|-L|-v[v]|<file>\n", prog);
	fprintf (to, "\t-a lists alternatives.\n", prog);
	fprintf (to, "\t-l lists misspelings.\n", prog);
	fprintf (to, "\t-L displays line numbers.\n", prog);
	fprintf (to, "\t-v displays program version.\n", prog);
}

static gboolean
consume_line (FILE * in, GString * str)
{
	int ch;
	gsize bytes_read, bytes_written;
	gchar * utf;
	gboolean ret = TRUE;

	g_string_truncate (str, 0);

	while (ret && (ch = fgetc (in)) != EOF) {
		if (ch == '\r')
			continue;
		else {
			g_string_append_c (str, ch);
			if (ch == '\n')
				ret = FALSE;
		}
	}

	if (str->len) {
		utf = g_locale_to_utf8 (str->str, str->len, &bytes_read, &bytes_written, NULL);
		g_string_truncate (str, 0);

		if (utf) {
			g_string_assign (str, utf);
			g_free (utf);
		}
	}

	return ret;
}

static void
print_utf (FILE * out, const char * str)
{
	gsize bytes_read, bytes_written;
	gchar * native;

	native = g_locale_from_utf8 (str, -1, &bytes_read, &bytes_written, NULL);
	if (native) {
		fwrite (native, 1, bytes_written, out);
		g_free (native);
	}
}

static void
do_mode_a (FILE * out, EnchantDict * dict, GString * word, size_t start_pos, size_t lineCount)
{
	size_t n_suggs;
	char ** suggs;	

	if (word->len <= MIN_WORD_LENGTH || enchant_dict_check (dict, word->str, word->len) == 0)
		if (lineCount)
			fprintf (out, "* %ld\n", lineCount);
		else
			fwrite ("*\n", 1, 2, out);
	else {
		suggs = enchant_dict_suggest (dict, word->str, 
					      word->len, &n_suggs);
		if (!n_suggs || !suggs) {
			fwrite ("# ", 1, 2, out);
			if (lineCount)
				fprintf (out, "%ld ", lineCount);
			print_utf (out, word->str);
			fprintf (out, " %ld\n", start_pos+1);
		}
		else {
			size_t i = 0;
			
			fwrite ("& ", 1, 2, out);
			if (lineCount)
				fprintf (out, "%ld ", lineCount);
			print_utf (out, word->str);
			fprintf (out, " %ld %ld:", n_suggs, start_pos);
			
			for (i = 0; i < n_suggs; i++) {
				fprintf (out, " ");
				print_utf (out, suggs[i]);

				if (i != (n_suggs - 1))
					fwrite (",", 1, 1, out);
				else
					fwrite ("\n", 1, 1, out);
			}
		}
	}
}

static void
do_mode_l (FILE * out, EnchantDict * dict, GString * word, size_t lineCount)
{
	if (enchant_dict_check (dict, word->str, word->len) != 0) {
		if (lineCount)
			fprintf (out, "%ld ", lineCount);
		print_utf (out, word->str);
		fwrite ("\n", 1, 1, out);
	}
}

/* splits a line into a set of (word,word_position) touples */
static GSList *
tokenize_line (GString * line)
{
	GSList * tokens = NULL;
	size_t start_pos, cur_pos;
	char *utf = (char *) line->str;

	GString * word;
	
	gunichar uc;
	
	start_pos = cur_pos = 0;
	word = g_string_new (NULL);

	while (cur_pos < line->len && *utf) {
		uc = g_utf8_get_char (utf); 
		
		switch (g_unichar_type(uc)) {
		case G_UNICODE_MODIFIER_LETTER:
		case G_UNICODE_LOWERCASE_LETTER:
		case G_UNICODE_TITLECASE_LETTER:
		case G_UNICODE_UPPERCASE_LETTER:
		case G_UNICODE_OTHER_LETTER:
		case G_UNICODE_COMBINING_MARK:
		case G_UNICODE_ENCLOSING_MARK:
		case G_UNICODE_NON_SPACING_MARK:
		case G_UNICODE_DECIMAL_NUMBER:
		case G_UNICODE_LETTER_NUMBER:
		case G_UNICODE_OTHER_NUMBER:
		case G_UNICODE_CONNECT_PUNCTUATION:
			g_string_append_unichar (word, uc);
			cur_pos++;
			break;
		case G_UNICODE_OTHER_PUNCTUATION:
			if (uc == '\'') {
				g_string_append_unichar (word, uc);
				cur_pos++;
				break;
			}
			/* else fall through */
		default: /* some sort of non-word character */
			if (word->len) {
				tokens = g_slist_append (tokens,
							 g_string_new_len (word->str, word->len));
				tokens = g_slist_append (tokens,
							 GINT_TO_POINTER(start_pos));
				g_string_truncate (word, 0);
				start_pos = ++cur_pos;
			}
			break;
		}
		utf = g_utf8_next_char (utf);
	}

	g_string_free (word, TRUE);

	return tokens;
}

static int
parse_file (FILE * in, FILE * out, IspellMode_t mode, int countLines)
{
	EnchantBroker * broker;
	EnchantDict * dict;
	
	GString * str, * word = NULL;
	GSList * tokens, *token_ptr;
	gchar * lang, *lang_punct;
	size_t pos, lineCount = 0;

	gboolean was_last_line = FALSE, corrected_something = FALSE;

	if (mode == MODE_A)
		print_version (out);

	lang = g_strdup (g_getenv ("LANG"));
	if (!lang || !strcmp (lang, "C"))
		lang = g_strdup ("en");
	else {
		/* get rid of useless trailing garbage like de_DE@euro or de_DE.ISO-8859-15 */
		if ((lang_punct = strrchr (lang, '.')) != NULL)
			*lang_punct = '\0';
		if ((lang_punct = strrchr (lang, '@')) != NULL)
			*lang_punct = '\0';
	}
	
	broker = enchant_broker_init ();
	dict = enchant_broker_request_dict (broker, lang);

	if (!dict) {
		fprintf (stderr, "Couldn't create a dictionary for %s\n", lang);
		g_free (lang);
		enchant_broker_free (broker);
		return 1;
	}

	g_free (lang);

	str = g_string_new (NULL);
	
	while (!was_last_line) {
		was_last_line = consume_line (in, str);

		if (countLines)
			lineCount++;

		if (str->len) {

			corrected_something = FALSE;
			token_ptr = tokens = tokenize_line (str);
			while (tokens != NULL) {
				corrected_something = TRUE;

				word = (GString *)tokens->data;
				tokens = tokens->next;
				pos = GPOINTER_TO_INT(tokens->data);
				tokens = tokens->next;

				if (mode == MODE_A)
					do_mode_a (out, dict, word, pos, lineCount);
				else if (mode == MODE_L)
					do_mode_l (out, dict, word, lineCount);
			}

			if (token_ptr)
				g_slist_free (token_ptr);
		} 
		
		if (mode == MODE_A && corrected_something)
			fwrite ("\n", 1, 1, out);
		
		g_string_truncate (str, 0);
	}
	
	enchant_broker_free_dict (broker, dict);
	enchant_broker_free (broker);

	if (word)
		g_string_free (word, TRUE);
	g_string_free (str, TRUE);

	return 0;
}

int main (int argc, char ** argv)
{
	IspellMode_t mode = MODE_NONE;
	
	char * arg, * file = NULL;
	int i, rval = 0;
	
	FILE * fp = stdin;

	int countLines = 0;
	
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
				else if (arg[1] == 'L')
					countLines = 1;
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
		
		rval = parse_file (fp, stdout, mode, countLines);
		
		if (file)
			fclose (fp);
	}
	
	return rval;
}
