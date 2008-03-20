/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2003 Dom Lachowicz
 *               2007 Hannu Väisänen
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
 * file, but you are not obligated to do so. If you do not wish to
 * do so, delete this exception statement from your version.
 */

/**
 * This is a rough approximation of an "ispell compatibility mode"
 * for Enchant.
 *
 * Modified in 2007 to work when called from emacs which
 * calls a spelling program (e.g. enchant) like this
 *
 * enchant -a -m -d dictionary
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "enchant.h"
#include "enchant-provider.h"

/* word has to be bigger than this to be checked */
#define MIN_WORD_LENGTH 1


typedef enum 
	{
		MODE_NONE,
		MODE_VERSION,
		MODE_A,
		MODE_L
	} IspellMode_t;

static void 
print_version (FILE * to)
{
	fprintf (to, "@(#) International Ispell Version 3.1.20 (but really Enchant %s)\n", VERSION);
	fflush (to);
}

static void
print_help (FILE * to, const char * prog)
{
	fprintf (to, "Usage: %s [options] -a|-d dict|-l|-L|-m|-v[v]|<file>\n", prog);
	fprintf (to, "\t-a lists alternatives.\n");
	fprintf (to, "\t-d dict uses dictionary <dict>.\n");
	fprintf (to, "\t-l lists misspellings.\n");
	fprintf (to, "\t-m is ignored.\n");
	fprintf (to, "\t-L displays line numbers.\n");
	fprintf (to, "\t-v displays program version.\n");
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

		if (utf) {
			g_string_assign (str, utf);
			g_free (utf);
		} 
		/* Else str->str stays the same. we'll assume that it's 
		   already utf8 and glib is just being stupid. */
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
	} else {
		/* We'll assume that it's already utf8 and glib is just being stupid. */
		fwrite (str, 1, strlen (str), out);
	}
}

static void
do_mode_a (FILE * out, EnchantDict * dict, GString * word, size_t start_pos, size_t lineCount)
{
	size_t n_suggs;
	char ** suggs;	

	if (word->len <= MIN_WORD_LENGTH || enchant_dict_check (dict, word->str, word->len) == 0) {
		if (lineCount)
			fprintf (out, "* %ld\n", lineCount);
		else
			fwrite ("*\n", 1, 2, out);
	}
	else {
		suggs = enchant_dict_suggest (dict, word->str, 
					      word->len, &n_suggs);
		if (!n_suggs || !suggs) {
			fwrite ("# ", 1, 2, out);
			if (lineCount)
				fprintf (out, "%ld ", lineCount);
			print_utf (out, word->str);
			fprintf (out, " %ld\n", start_pos);
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

			enchant_dict_free_string_list (dict, suggs);
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


static int
is_word_char (gunichar uc, size_t n)
{
	GUnicodeType type;

	type = g_unichar_type(uc);

	switch (type) {
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
                return 1;     /* Enchant 1.3.0 defines word chars like this. */

	case G_UNICODE_CONTROL:
	case G_UNICODE_FORMAT:
	case G_UNICODE_UNASSIGNED:
	case G_UNICODE_PRIVATE_USE:
	case G_UNICODE_SURROGATE:
	case G_UNICODE_DASH_PUNCTUATION:
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
		if ((n > 0) && (uc == g_utf8_get_char("'"))) {
		        return 1;  /** Char ' is accepted only within a word. */
		}
		else if ((n > 0) && (type == G_UNICODE_DASH_PUNCTUATION)) {
			return 1; /* hyphens only accepted within a word. */
		}

		return 0;
	}
}


typedef struct lang_map {
  const char *ispell;
  const char *enchant;
} LangMap;


/* Maps ispell language codes to enchant language codes. */
/* The list is partially taken from src/ispell/ispell_checker.cpp. */
static const LangMap lingua[] = {
	{"american",	"en_US"},
	{"brazilian",	"pt_BR"},
	{"british",	"en_GB"},
	{"bulgarian",	"bg"},
	{"catala",	"ca"},
	{"catalan",	"ca"},
	{"danish",	"da"},
	{"dansk",	"da"},
	{"deutsch",	"de"},
	{"dutch",	"nl"},
	{"ellhnika",	"el"},
	{"espanol",	"es"},
	{"esperanto",	"eo"},
	{"estonian",	"et"},
	{"faeroese",	"fo"},
	{"finnish",	"fi"},
	{"francais",	"fr"},
	{"french",	"fr"},
	{"galician",	"gl"},
	{"german",	"de"},
	{"hungarian",	"hu"},
	{"interlingua",	"ia"},
	{"irish",	"ga"},
	{"italian",	"it"},
	{"latin",	"la"},
	{"lietuviu",	"lt"},
	{"lithuanian",	"lt"},
	{"mlatin",	"la"},
	{"nederlands",	"nl"},
	{"norsk",	"no"},
	{"norwegian",	"no"},
	{"nynorsk",	"nn"},
	{"polish",	"pl"},
	{"portugues",	"pt"},
	{"portuguese",	"pt"},
	{"russian",	"ru"},
	{"sardinian",	"sc"},
	{"slovak",	"sk"},
	{"slovenian",	"sl"},
	{"slovensko",	"sl"},
	{"spanish",	"es"},
	{"suomi",	"fi"},   /* For Emacs/Voikko/tmispell compatibility. */
	{"svenska",	"sv"},
	{"swedish",	"sv"},
	{"swiss",	"de_CH"},
	{"ukrainian",	"uk"},
	{"yiddish-yivo",	"yi"},
	{NULL,       NULL}    /* Last item must be {NULL, NULL}. */
};


/* Converts ispell language code to enchant language code. */
static gchar *
convert_language_code (gchar *code)
{
	size_t i;
	for (i = 0; lingua[i].ispell; i++) {
	        if (!strcmp(code,lingua[i].ispell)) {
			/* We must call g_strdup() because the calling program g_free()s the result. */
		        return g_strdup (lingua[i].enchant);
		}
	}
	/* Let's call g_strdup() here too! */
	return g_strdup (code);
}


/* Splits a line into a set of (word,word_position) touples. */
static GSList *
tokenize_line (GString * line)
{
	GSList * tokens = NULL;
	char *utf = (char *) line->str;

	GString * word;
	
	gunichar uc;
	size_t cur_pos = 0;
	size_t start_pos = 0;
	word = g_string_new (NULL);

	while (cur_pos < line->len && *utf) {
    int i;
	        /* Skip non-word characters. */
		cur_pos = g_utf8_pointer_to_offset ((const char*)line->str, utf);
		uc = g_utf8_get_char (utf);
		while (cur_pos < line->len && *utf && !is_word_char(uc,0)) {
		        utf = g_utf8_next_char (utf);
			uc = g_utf8_get_char (utf);
			cur_pos = g_utf8_pointer_to_offset ((const char*)line->str, utf);
		}
		start_pos = cur_pos;

		/* Skip over word. */
		while (cur_pos < line->len && *utf && is_word_char(uc,1)) {
			g_string_append_unichar (word, uc);
		        utf = g_utf8_next_char (utf);
			uc = g_utf8_get_char (utf);
			cur_pos = g_utf8_pointer_to_offset ((const char*)line->str, utf);
		}

	        /* Do not accept one or more  ' at the end of the word. */
		i = word->len-1;
	        while ((i >= 0) && (word->str[i] == '\'')) {
	                g_string_truncate (word, i);
			i--;
		}

		/* Save (word, position) touple. */
                if (word->len) {
		        tokens = g_slist_append (tokens, g_string_new_len (word->str, word->len));
			tokens = g_slist_append (tokens, GINT_TO_POINTER(start_pos));
			g_string_truncate (word, 0);
		}
	}
	g_string_free (word, TRUE);

	return tokens;
}

static int
parse_file (FILE * in, FILE * out, IspellMode_t mode, int countLines, gchar *dictionary)
{
	EnchantBroker * broker;
	EnchantDict * dict;
	
	GString * str, * word = NULL;
	GSList * tokens, *token_ptr;
	gchar * lang;
	size_t pos, lineCount = 0;

	gboolean was_last_line = FALSE, corrected_something = FALSE;

	if (mode == MODE_A)
		print_version (out);

	if (dictionary) {
		lang = convert_language_code (dictionary);
	}
	else {
	        lang = enchant_get_user_language();
		if(!lang)
			return 1;
 	}

	/* Enchant will get rid of useless trailing garbage like de_DE@euro or de_DE.ISO-8859-15 */
	
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
		
		if (mode == MODE_A && corrected_something) {
			fwrite ("\n", 1, 1, out);
		}
		g_string_truncate (str, 0);
		fflush (out);
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
	
	char * file = NULL;
	int i, rval = 0;
	
	FILE * fp = stdin;

	int countLines = 0;
	gchar *dictionary = 0;  /* -d dictionary */


	for (i = 1; i < argc; i++) {
		char * arg = argv[i];
		if (arg[0] == '-') {
			if (strlen (arg) == 2) {
				/* It seems that the first one of these that is specified gets precedence. */
				if (arg[1] == 'a' && MODE_NONE == mode)
					mode = MODE_A;
				else if (arg[1] == 'l' && MODE_NONE == mode)
					mode = MODE_L;
				else if (arg[1] == 'v' && MODE_NONE == mode)
					mode = MODE_VERSION;
				else if (arg[1] == 'L' && MODE_NONE == mode)
					countLines = 1;
				else if (arg[1] == 'm')
				     	; /* Ignore. Emacs calls ispell with '-m'. */
				else if (arg[1] == 'd') {
				     	i++;
					dictionary = argv[i];  /* Emacs calls ispell with '-d dictionary'. */
				}
			} 
			else if ((strlen (arg) == 3) && (arg[1] == 'v') && (arg[2] == 'v')) {
			     	mode = MODE_VERSION;   /* Emacs (or ispell.el) calls [ai]spell with '-vv'. */
			}
			else if (arg[1] == 'd') {
			        dictionary = arg + 2;  /* Accept "-ddictionary", i.e. no space between -d and dictionary. */
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
			fp = g_fopen (file, "rb");
			if (!fp) {
				fprintf (stderr, "Error: Could not open the file \"%s\" for reading.\n", file);
				exit (1);
			}
		}
		
		rval = parse_file (fp, stdout, mode, countLines, dictionary);
		
		if (file)
			fclose (fp);
	}
	
	return rval;
}
