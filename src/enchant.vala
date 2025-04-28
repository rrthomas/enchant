#! /usr/bin/env -S vala --vapidir src --vapidir lib --pkg config --pkg configmake --pkg posix --pkg enchant-3 --pkg util
/* enchant: List misspellings in a file.
 *
 * Copyright (C) 2003 Dom Lachowicz
 *               2007 Hannu Väisänen
 *               2016-2025 Reuben Thomas
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders
 * give permission to link the code of this program with
 * the non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers. If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so. If you do not wish to
 * do so, delete this exception statement from your version.
 */

using Enchant;

using Posix;

/* word has to be bigger than this to be checked */
const uint MIN_WORD_LENGTH = 1;

string charset;

string? get_line(FileStream fin) {
	string str = fin.read_line();
	if (str != null && str.length > 0) {
		try {
			return convert(str, str.length, "UTF-8", charset);
		} catch (ConvertError e) {
			/* Assume that str is already utf8 and glib is just being stupid. */
		}
	}
	return str;
}

void print_utf(string str) {
	size_t bytes_written;
	try {
		string native = str.locale_from_utf8(str.length, null, out bytes_written);
		/* Print arbitrary bytes (including potential NULs). */
		unowned uint8[] buf = (uint8[]) native;
		buf.length = (int)bytes_written;
		GLib.stdout.write(buf);
	} catch (GLib.ConvertError e) {
		/* Assume that it's already utf8 and glib is just being stupid. */
		print("%s", str);
	}
}

bool check_word(Dict dict, string word) {
	return word.length <= MIN_WORD_LENGTH ||
		   dict.check(word, word.length) == 0;
}

void check_line(Dict dict, string word, size_t line_count) {
	if (!check_word(dict, word)) {
		if (line_count > 0)
			print("%zu ", line_count);
		print_utf(word);
		GLib.stdout.putc('\n');
	}
}


/* Splits a line into a list of words. */
SList<string> tokenize_line(Dict dict, string line) {
	var tokens = new SList<string>();
	long cur_unichar = 0;

	for (unowned string cur_byte = line; cur_byte[0] != '\0';) {
		var word = new StringBuilder();
		unichar uc;

		/* Skip non-word characters. */
		for (uc = cur_byte.get_char();
			 uc != 0 && !dict.is_word_character(uc, WordPosition.START);
			 uc = cur_byte.get_char()) {
			cur_byte = cur_byte.next_char();
			cur_unichar += 1;
		}
		var start_ptr = cur_byte;

		/* Skip over word characters. */
		for (;
			 uc != 0 && dict.is_word_character(uc, WordPosition.MIDDLE);
			 uc = cur_byte.get_char()) {
			cur_byte = cur_byte.next_char();
			word.append_unichar(uc);
			cur_unichar += 1;
		}

		/* Skip backwards over any characters that can't appear at the end of a word. */
		unowned string last_char_ptr = cur_byte;
		for (;
			 !dict.is_word_character(last_char_ptr.get_char(), WordPosition.END);
			 last_char_ptr = last_char_ptr.prev_char()) ;
		word.truncate((char *) last_char_ptr.next_char() - (char *) start_ptr);

		/* Save (word, position) tuple. */
		if (word.len > 0) {
			tokens.append(word.str);
		}
	}

	return tokens;
}

void usage(OptionContext ctx) {
	print("%s", ctx.get_help(false, null));
	exit(1);
}

void describe_dict(string lang_tag,
				   string provider_name,
				   string provider_desc,
				   string provider_file) {
	print("%s (%s)\n", lang_tag, provider_name);
}

void describe_word_chars(string lang_tag,
						 string provider_name,
						 string provider_desc,
						 string provider_file,
						 Dict self) {
	string word_chars = "";
	if (self != null)
		word_chars = self.get_extra_word_characters();
	print("%s\n", word_chars != null ? word_chars : "");
}

void describe_provider(string name, string desc, string file) {
	print("%s (%s)\n", name, desc);
}

string get_user_language() {
	// The returned list always contains "C".
	unowned string[] languages = Intl.get_language_names();
	GLib.assert(languages != null);
	return languages[0];
}

public class Main : Object {
	private static string dictionary = null;
	private static string perslist = null;
	[CCode (array_length = false, array_null_terminated = true)]
	private static string[] files; /* FILE... */
	private static bool version = false;
	private static bool count_lines = false;
	private static bool list_providers = false;
	private static bool list_dictionaries = false;
	private static bool check_spelling = false;
	private static bool show_default_dict = false;
	private static bool show_word_chars = false;

	private const OptionEntry[] main_options = {
		{"dictionary", 'd', OptionFlags.NONE, OptionArg.STRING, ref dictionary, "Use the given dictionary", "DICTIONARY"},
		{"check-spelling", 'l', OptionFlags.NONE, OptionArg.NONE, ref check_spelling, "List misspellings in the input files, or standard input", null},
		{"pwl", 'p', OptionFlags.NONE, OptionArg.FILENAME, ref perslist, "Use the given personal word list", "FILE"},
		{"show-lines", 'L', OptionFlags.NONE, OptionArg.NONE, ref count_lines, "Display line numbers", null},
		{"list-providers", '\0', OptionFlags.NONE, OptionArg.NONE, ref list_providers, "List spelling providers", null},
		{"list-dicts", '\0', OptionFlags.NONE, OptionArg.NONE, ref list_dictionaries, "List all dictionaries", null},
		{"default-dict", '\0', OptionFlags.NONE, OptionArg.NONE, ref show_default_dict, "Show the default dictionary for the given or default language", null},
		{"word-chars", '\0', OptionFlags.NONE, OptionArg.NONE, ref show_word_chars, "Show the word characters for the given or default language", null},
		{"version", 'v', OptionFlags.NONE, OptionArg.NONE, ref version, "Display version information and exit", null},

		/* Files */
		{OPTION_REMAINING, '\0', OptionFlags.NONE, OptionArg.FILENAME_ARRAY, ref files, null, "FILE..."},
		{null}
	};

	private static bool check_file(Broker broker, Dict dict, FileStream fin) {
		size_t line_count = 0;
		string str;
		while ((str = get_line(fin)) != null) {
			if (count_lines)
				line_count++;

			if (str.length > 0) {
				var tokens = tokenize_line(dict, str);
				if (tokens == null)
					GLib.stdout.putc('\n');
				for (unowned var tok_ptr = tokens; tok_ptr != null; tok_ptr = tok_ptr.next) {
					check_line(dict, tok_ptr.data, line_count);
				}
			}

			GLib.stdout.flush();
		}

		return true;
	}

	public static int main(string[] args) {
		/* Initialize system locale. */
		Intl.setlocale();
		get_charset(out charset);
		var no_args = args.length <= 1;

		/* Parse command line arguments. */
		var ctx = new OptionContext("\n\nCheck spelling non-interactively.");
		ctx.set_help_enabled(true);
		ctx.add_main_entries(main_options, null);
		try {
			ctx.parse(ref args);
		} catch (OptionError e) {
			printerr("%s-%s: %s\n", PACKAGE, ENCHANT_MAJOR_VERSION, e.message);
			usage(ctx);
		}
		if (no_args) {
			usage(ctx);
		}
		if (version) {
			print("%s\n", PACKAGE_STRING);
			exit(0);
		}

		/* Ensure we have a language set. */
		if (dictionary == null) {
			dictionary = get_user_language();
			if (dictionary == "C")
				dictionary = "en";
		}

		/* Initialise a broker and request a dictionary. */
		var broker = new Broker();
		unowned var dict = broker.request_dict_with_pwl(dictionary, perslist);
		if (dict == null) {
			GLib.stderr.printf("No dictionary available for '%s'", dictionary);
			string errmsg = broker.get_error();
			if (errmsg != null)
				GLib.stderr.printf(": %s", errmsg);
			GLib.stderr.putc('\n');
			exit(1);
		}

		/* Perform requested actions. */
		if (list_providers) {
			broker.describe(describe_provider);
		}
		if (list_dictionaries) {
			broker.list_dicts(describe_dict);
		}
		if (show_default_dict || show_word_chars) {
			DictDescribeFn fn;
			if (show_default_dict)
				fn = describe_dict;
			else
				fn = describe_word_chars;
			dict.describe(fn, dict);
		}
		if (check_spelling) {
			/* Process the files or standard input. */
			FileStream fp = null;
			if (files == null)
				return check_file(broker, dict, GLib.stdin) ? 0 : 1;
			foreach (var f in files) {
				fp = FileStream.open(f, "rb");
				if (fp == null) {
					GLib.stderr.printf("Error: Could not open the file \"%s\" for reading.\n", f);
					exit(1);
				}
				if (!check_file(broker, dict, fp))
					exit(1);
			}
		}
		return 0;
	}
}
