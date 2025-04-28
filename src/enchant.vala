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


/* Splits a line into a set of (word,word_position) tuples. */
class Token {
	public string word;
	public long pos;

	public Token(string word, long pos) {
		this.word = word;
		this.pos = pos;
	}
}

SList<Token> tokenize_line(Dict dict, string line) {
	var tokens = new SList<Token>();
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
		long start_unichar = cur_unichar;

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
			tokens.append(new Token(word.str, start_unichar));
		}
	}

	return tokens;
}

void usage(OptionContext ctx) {
	print("%s", ctx.get_help(false, null));
	exit(1);
}

public class Main : Object {
	private static string dictionary = null;  /* -d dictionary */
	private static string perslist = null; /* -p personal_word_list */
	[CCode (array_length = false, array_null_terminated = true)]
	private static string[] files; /* FILE... */
	private static bool version = false;
	private static bool count_lines = false;

	private const OptionEntry[] main_options = {
		{"dictionary", 'd', OptionFlags.NONE, OptionArg.STRING, ref dictionary, "Use the given dictionary", "DICTIONARY"},
		{"pwl", 'p', OptionFlags.NONE, OptionArg.FILENAME, ref perslist, "Use the given personal word list", "FILE"},
		{"show-lines", 'L', OptionFlags.NONE, OptionArg.NONE, ref count_lines, "Display line numbers", null},
		{"version", 'v', OptionFlags.NONE, OptionArg.NONE, ref version, "Display version information and exit", null},

		/* Files */
		{OPTION_REMAINING, '\0', OptionFlags.NONE, OptionArg.FILENAME_ARRAY, ref files, null, "FILE..."},
		{null}
	};

	private static bool parse_file(FileStream fin) {
		string lang;
		if (dictionary != null)
			lang = dictionary;
		else {
			lang = enchant_get_user_language();
			if (lang == null)
				return false;
			if (lang == "C")
				lang = "en";
		}

		var broker = new Broker();
		unowned var dict = broker.request_dict_with_pwl(lang, perslist);

		if (dict == null) {
			GLib.stderr.printf("No dictionary available for '%s'", lang);
			string errmsg = broker.get_error();
			if (errmsg != null)
				GLib.stderr.printf(": %s", errmsg);
			GLib.stderr.putc('\n');

			return false;
		}

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
					check_line(dict, tok_ptr.data.word, line_count);
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
		if (version) {
			print("%s\n", PACKAGE_STRING);
			exit(0);
		}

		/* Process the file or standard input. */
		FileStream fp = null;
		if (files == null)
			return parse_file(GLib.stdin) ? 0 : 1;
		foreach (var f in files) {
			fp = FileStream.open(f, "rb");
			if (fp == null) {
				GLib.stderr.printf("Error: Could not open the file \"%s\" for reading.\n", f);
				exit(1);
			}
			if (!parse_file(fp))
				exit(1);
		}
		return 0;
	}
}
