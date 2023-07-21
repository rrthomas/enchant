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

/**
 *  This file implements personal word list (PWL) dictionaries in the
 *  type EnchantPWL.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>

#include <glib.h>
#include <glib/gstdio.h>
#include "enchant-provider.h"

#include "pwl.h"

static const gunichar BOM = 0xfeff;

/*  A PWL dictionary is stored as a GHashTable of UTF-8 strings.
 */
struct str_enchant_pwl
{
	char * filename;
	time_t file_changed;
	GHashTable *words;
};

/*
 *   Function Prototypes
 */

static void enchant_pwl_add_to_table(EnchantPWL *pwl,
					const char *const word, size_t len);
static void enchant_pwl_refresh_from_file(EnchantPWL* pwl);

#define enchant_lock_file(f) flock (fileno (f), LOCK_EX)
#define enchant_unlock_file(f) flock (fileno (f), LOCK_UN)

/**
 * enchant_pwl_init
 *
 * Returns: a new PWL object used to store/check words.
 */
EnchantPWL* enchant_pwl_init(void)
{
	EnchantPWL *pwl = g_new0(EnchantPWL, 1);
	pwl->words = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

	return pwl;
}

/**
 * enchant_pwl_init_with_file
 *
 * Returns: a new PWL object used to store/check words
 * or NULL if the file cannot be opened or created
 */
EnchantPWL* enchant_pwl_init_with_file(const char * file)
{
	g_return_val_if_fail (file != NULL, NULL);

	FILE* fd = g_fopen(file, "a+");
	if(fd == NULL)
		return NULL;
	fclose(fd);
	EnchantPWL *pwl = enchant_pwl_init();
	pwl->filename = g_strdup(file);
	pwl->file_changed = 0;

	enchant_pwl_refresh_from_file(pwl);
	return pwl;
}

static void enchant_pwl_refresh_from_file(EnchantPWL* pwl)
{
	GStatBuf stats;
	if(!pwl->filename ||
	   g_stat(pwl->filename, &stats) != 0 || /* presumably I won't be able to open the file either */
	   pwl->file_changed == stats.st_mtime) /* nothing changed since last read */
		return;

	g_hash_table_destroy (pwl->words);
	pwl->words = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

	FILE *f = g_fopen(pwl->filename, "r");
	if (!f)
		return;

	pwl->file_changed = stats.st_mtime;

	enchant_lock_file (f);

	char buffer[BUFSIZ + 1];
	size_t line_number = 1;
	for (; NULL != (fgets (buffer, sizeof (buffer), f)); ++line_number)
		{
			char *line = buffer;
			if(line_number == 1 && BOM == g_utf8_get_char(line))
				line = g_utf8_next_char(line);

			if(line[strlen(line)-1] != '\n' && !feof(f)) /* ignore lines longer than BUFSIZ. */
				{
					g_warning ("Line too long (ignored) in %s at line:%zu\n", pwl->filename, line_number);
					while (NULL != (fgets (buffer, sizeof (buffer), f)))
						{
							if (line[strlen(buffer)-1]=='\n')
								break;
						}
					continue;
				}

			g_strchomp(line);
			if( line[0] && line[0] != '#')
				{
					if(g_utf8_validate(line, -1, NULL))
						enchant_pwl_add_to_table(pwl, line, strlen(line));
					else
						g_warning ("Bad UTF-8 sequence in %s at line:%zu\n", pwl->filename, line_number);
				}
		}

	enchant_unlock_file (f);
	fclose (f);
}

void enchant_pwl_free(EnchantPWL *pwl)
{
	g_free(pwl->filename);
	g_hash_table_destroy (pwl->words);
	g_free(pwl);
}

static void enchant_pwl_add_to_table(EnchantPWL *pwl,
					const char *const word, size_t len)
{
	char * normalized_word = g_utf8_normalize (word, len, G_NORMALIZE_NFD);
	if(NULL != g_hash_table_lookup (pwl->words, normalized_word)) {
		g_free (normalized_word);
		return;
	}

	g_hash_table_insert (pwl->words, normalized_word, g_strndup(word,len));
}

static void enchant_pwl_remove_from_table(EnchantPWL *pwl,
					const char *const word, size_t len)
{
	char * normalized_word = g_utf8_normalize (word, len, G_NORMALIZE_NFD);
	g_hash_table_remove (pwl->words, normalized_word);
	g_free(normalized_word);
}

void enchant_pwl_add(EnchantPWL *pwl,
			 const char *const word, ssize_t len)
{
	if (len < 0)
		len = strlen (word);

	enchant_pwl_refresh_from_file(pwl);

	enchant_pwl_add_to_table(pwl, word, len);

	if (pwl->filename != NULL)
	{
		FILE *f = g_fopen(pwl->filename, "a+");
		if (f)
			{
				/* Since this function does not signal I/O
				   errors, only use return values to avoid
				   doing things that seem futile. */

				enchant_lock_file (f);
				GStatBuf stats;
				if(g_stat(pwl->filename, &stats)==0)
					pwl->file_changed = stats.st_mtime;

				/* Add a newline if the file doesn't end with one. */
				if (fseek (f, -1, SEEK_END) == 0)
					{
						int c = getc (f);
						fseek (f, 0L, SEEK_CUR); /* ISO C requires positioning between read and write. */
						if (c != '\n')
							putc ('\n', f);
					}

				if (fwrite (word, sizeof(char), len, f) == (size_t)len)
					putc ('\n', f);
				enchant_unlock_file (f);
				fclose (f);
			}
	}
}

void enchant_pwl_remove(EnchantPWL *pwl,
			 const char *const word, ssize_t len)
{
	if (len < 0)
		len = strlen (word);

	if(enchant_pwl_check(pwl, word, len) == 1)
		return;

	enchant_pwl_refresh_from_file(pwl);

	enchant_pwl_remove_from_table(pwl, word, len);

	if (pwl->filename)
		{
			char * contents;
			size_t length;

			if(!g_file_get_contents(pwl->filename, &contents, &length, NULL))
				return;

			FILE *f = g_fopen(pwl->filename, "wb"); /*binary because g_file_get_contents reads binary*/
			if (f)
				{
					enchant_lock_file (f);
					char *key = g_strndup(word, len);

					char *filestart;
					if(BOM == g_utf8_get_char(contents))
						{
							filestart = g_utf8_next_char(contents);
							fwrite (contents, sizeof(char), filestart-contents, f);
						}
					else
						filestart = contents;

					char *searchstart = filestart;
					for(;;)
						{
							/*find word*/
							char *needle = strstr(searchstart, key);
							if(needle == NULL)
								{
									fwrite (searchstart, sizeof(char), length - (searchstart - contents), f);
									break;
								}
							else
								{
									char* foundend = needle+len;
									if((needle == filestart || contents[needle-contents-1] == '\n' || contents[needle-contents-1] == '\r') &&
										(foundend == contents + length || *foundend == '\n' || *foundend == '\r'))
										{
											fwrite (searchstart, sizeof(char), needle - searchstart, f);
											searchstart = foundend;
											while (*searchstart == '\n' || *searchstart == '\r')
												++searchstart;
										}
									else {
										fwrite (searchstart, sizeof(char), needle - searchstart+1, f);
										searchstart = needle+1;
									}
								}
						}
					g_free(key);

					GStatBuf stats;
					if(g_stat(pwl->filename, &stats)==0)
						pwl->file_changed = stats.st_mtime;

					enchant_unlock_file (f);

					fclose (f);
				}
			g_free(contents);
		}
}

static int enchant_pwl_contains(EnchantPWL *pwl, const char *const word, size_t len)
{
	char * normalized_word = g_utf8_normalize (word, len, G_NORMALIZE_NFD);
	int found = NULL != g_hash_table_lookup (pwl->words, normalized_word);
	g_free(normalized_word);
	return found;
}

static int enchant_is_all_caps(const char*const word, size_t len)
{
	g_return_val_if_fail (word && *word, 0);

	int hasCap = 0;
	for (const char *it = word; it < word + len; it = g_utf8_next_char(it))
		{
			GUnicodeType type = g_unichar_type(g_utf8_get_char(it));
			switch(type)
				{
				case G_UNICODE_UPPERCASE_LETTER:
					hasCap = 1;
					break;
				case G_UNICODE_TITLECASE_LETTER:
				case G_UNICODE_LOWERCASE_LETTER:
					return 0;

				case G_UNICODE_CONTROL:
				case G_UNICODE_FORMAT:
				case G_UNICODE_UNASSIGNED:
				case G_UNICODE_PRIVATE_USE:
				case G_UNICODE_SURROGATE:
				case G_UNICODE_MODIFIER_LETTER:
				case G_UNICODE_OTHER_LETTER:
				case G_UNICODE_SPACING_MARK:
				case G_UNICODE_ENCLOSING_MARK:
				case G_UNICODE_NON_SPACING_MARK:
				case G_UNICODE_DECIMAL_NUMBER:
				case G_UNICODE_LETTER_NUMBER:
				case G_UNICODE_OTHER_NUMBER:
				case G_UNICODE_CONNECT_PUNCTUATION:
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
					break;
				}
		}

	return hasCap;
}

static G_GNUC_PURE int enchant_is_title_case(const char * const word, size_t len)
{
	g_return_val_if_fail (word && *word, 0);

	gunichar ch = g_utf8_get_char(word);
	GUnicodeType type = g_unichar_type(ch);
	if ((type != G_UNICODE_UPPERCASE_LETTER && type != G_UNICODE_TITLECASE_LETTER) ||
		ch != g_unichar_totitle(ch))
		return 0;

	for (const char* it = g_utf8_next_char(word); it < word + len; it = g_utf8_next_char(it))
		{
			type = g_unichar_type(g_utf8_get_char(it));
			if (type == G_UNICODE_UPPERCASE_LETTER || type == G_UNICODE_TITLECASE_LETTER)
				return 0;
		}

	return 1;
}

static gchar* enchant_utf8_strtitle(const gchar*str, gssize len)
{
	gchar *upperStr = g_utf8_strup(str, len); /* for locale-sensitive casing */
	gunichar title_case_char = g_unichar_totitle(g_utf8_get_char(upperStr));
	gchar title_case_utf8[7];
	gint utf8len = g_unichar_to_utf8(title_case_char, title_case_utf8);
	title_case_utf8[utf8len] = '\0';

	gchar *lowerTail = g_utf8_strdown(g_utf8_next_char(upperStr), -1);
	gchar *result = g_strconcat(title_case_utf8, lowerTail, NULL);
	g_free(upperStr);
	g_free(lowerTail);

	return result;
}

int enchant_pwl_check(EnchantPWL *pwl, const char *const word, ssize_t len)
{
	if (len < 0)
		len = strlen (word);

	enchant_pwl_refresh_from_file(pwl);

	int exists = enchant_pwl_contains(pwl, word, len);

	if(exists)
		return 0;

	int isAllCaps = 0;
	if(enchant_is_title_case(word, len) || (isAllCaps = enchant_is_all_caps(word, len)))
		{
			char * lower_case_word = g_utf8_strdown(word, len);
			exists = enchant_pwl_contains(pwl, lower_case_word, strlen(lower_case_word));
			g_free(lower_case_word);
			if(exists)
				return 0;

			if(isAllCaps)
			{
				char * title_case_word = enchant_utf8_strtitle(word, len);
				exists = enchant_pwl_contains(pwl, title_case_word, strlen(title_case_word));
				g_free(title_case_word);
				if(exists)
					return 0;
			}
		}

	return 1; /* not found */
}
