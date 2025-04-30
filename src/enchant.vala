/* enchant: An ispell-compatible command-line front-end for libenchant.
 *
 * Copyright (C) 2003 Dom Lachowicz
 *               2007 Hannu Väisänen
 *               2016-2024 Reuben Thomas
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

enum Mode	{
	NONE,
	A,
	L,
}

void print_version(FileStream to) {
	to.printf("@(#) International Ispell Version 3.1.20 (but really Enchant %s)\n", PACKAGE_VERSION);
	to.flush();
}

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

void do_mode_a(Dict dict, string word, size_t start_pos, size_t line_count, bool terse_mode) {
	if (check_word(dict, word)) {
		if (!terse_mode) {
			if (line_count > 0)
				print("* %zu\n", line_count);
			else
				print("*\n");
		}
	} else {
		string[] suggs = dict.suggest(word, word.length);
		if (suggs == null || suggs.length == 0) {
			print("# ");
			if (line_count > 0)
				print("%zu ", line_count);
			print_utf(word);
			print(" %zu\n", start_pos);
		} else {
			print("& ");
			if (line_count > 0)
				print("%zu ", line_count);
			print_utf (word);
			print(" %zu %zu:", suggs.length, start_pos);

			for (size_t i = 0; i < suggs.length; i++) {
				GLib.stdout.putc(' ');
				print_utf(suggs[i]);

				if (i != suggs.length - 1)
					GLib.stdout.putc(',');
			}
			GLib.stdout.putc('\n');
		}
	}
}

void do_mode_l(Dict dict, string word, size_t line_count) {
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
			 last_char_ptr = last_char_ptr.prev_char());
		word.truncate((char *) last_char_ptr.next_char() - (char *) start_ptr);

		/* Save (word, position) tuple. */
		if (word.len > 0) {
			tokens.append(new Token(word.str, start_unichar));
		}
	}

	return tokens;
}

errordomain Spelling {
	EMPTY_WORD,
	SYNTAX_ERROR,
}

void usage(OptionContext ctx) {
	print("%s", ctx.get_help(false, null));
	exit(1);
}

public class Main : Object {
	private static Mode mode = Mode.NONE;
	private static string dictionary = null;  /* -d dictionary */
	private static string perslist = null; /* -p personal_word_list */
	[CCode (array_length = false, array_null_terminated = true)]
	private static string[] files; /* FILE... */
	private static bool version = false;
	private static bool count_lines = false;
	private static bool ignored;

	private const OptionEntry[] main_options = {
		{"pipe", 'a', OptionFlags.NO_ARG, OptionArg.CALLBACK, (void *)Main.set_mode, "Talk to another program through a pipe, like Ispell", null},
		{"errors-only", 'l', OptionFlags.NO_ARG, OptionArg.CALLBACK, (void *)Main.set_mode, "List only the misspellings", null},
		{"dictionary", 'd', OptionFlags.NONE, OptionArg.STRING, ref dictionary, "Use the given dictionary", "DICTIONARY"},
		{"pwl", 'p', OptionFlags.NONE, OptionArg.FILENAME, ref perslist, "Use the given personal word list", "FILE"},
		{"show-lines", 'L', OptionFlags.NONE, OptionArg.NONE, ref count_lines, "Display line numbers", null},
		{"version", 'v', OptionFlags.NONE, OptionArg.NONE, ref version, "Display version information and exit", null},

		/* Ignore: Emacs calls ispell with '-m' and '-B'. */
		{" ", 'm', OptionFlags.HIDDEN, OptionArg.NONE, ref ignored, null, null},
		{" ", 'B', OptionFlags.HIDDEN, OptionArg.NONE, ref ignored, null, null},

		/* Files */
		{OPTION_REMAINING, '\0', OptionFlags.NONE, OptionArg.FILENAME_ARRAY, ref files, null, "FILE..."},
		{null}
	};

	private static bool set_mode(string option_name, string? val, void *data) {
		/* The first mode specified takes precedence. */
		if (mode == Mode.NONE) {
			if (option_name == "--pipe" || option_name == "-a")
				mode = Mode.A;
			else if (option_name == "--errors-only" || option_name == "-l")
				mode = Mode.L;
		}
		return true;
	}

	private static bool parse_file(FileStream fin) {
		var terse_mode = false;

		if (mode == Mode.A)
			print_version(GLib.stdout);

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

		var corrected_something = false;
		size_t line_count = 0;
		string str;
		while ((str = get_line(fin)) != null) {
			bool mode_A_no_command = false;

			if (count_lines)
				line_count++;

			if (str.length > 0) {
				corrected_something = false;

				if (mode == Mode.A) {
					try {
						switch (str[0]) {
						case '&': /* Insert uncapitalised in personal word list */
							if (str.length == 1)
								throw new Spelling.EMPTY_WORD("Word missing");
							if (str.length > 1) {
								unowned string new_word = str.next_char();
								unichar c = new_word.get_char_validated();
								if (c > 0) {
									dict.add(c.tolower().to_string() + new_word.next_char());
								} else
									dict.add(new_word);
							}
							break;
						case '*': /* Insert in personal word list */
							if (str.length == 1)
								throw new Spelling.EMPTY_WORD("Word missing");
							dict.add(str.next_char());
							break;
						case '@': /* Accept for this session */
							if (str.length == 1)
								throw new Spelling.EMPTY_WORD("Word missing");
							dict.add_to_session(str.substring(1), -1);
							break;
						case '/': /* Remove from personal word list */
							if (str.length == 1)
								throw new Spelling.EMPTY_WORD("Word missing");
							dict.remove(str.substring(1), -1);
							break;
						case '_': /* Remove from this session */
							if (str.length == 1)
								throw new Spelling.EMPTY_WORD("Word missing");
							dict.remove_from_session(str.substring(1), -1);
							break;

						case '%': /* Exit terse mode */
							terse_mode = false;
							break;
						case '!': /* Enter terse mode */
							terse_mode = true;
							break;

							/* Ignore these commands */
						case '#': /* Save personal word list (enchant does this automatically) */
						case '+': /* LaTeX mode */
						case '-': /* nroff mode [default] */
						case '~': /* change string character type (enchant is fixed to UTF-8) */
						case '`': /* Enter verbose-correction mode */
							break;

						case '$': /* Miscellaneous commands */
						{
							/* Save correction for rest of session [aspell extension] */
							if (str.has_prefix("$$ra ")) { /* Syntax: $$ra <MISSPELLED>,<REPLACEMENT> */
								// Enchant no longer supports this.
							} else if (str.has_prefix("$$wc"))
								/* Return the extra word chars list */
								print("%s\n", dict.get_extra_word_characters());
						}
						break;

						case '^': /* ^ is used as prefix to prevent interpretation of original
									 first character as a command */
						default: /* A word or words to check */
							mode_A_no_command = true;
							break;
						}
					} catch (Spelling e) {
						print("Error: The word \"\" is invalid. Empty string.\n");
					}
				}

				if (mode != Mode.A || mode_A_no_command) {
					var tokens = tokenize_line(dict, str);
					if (tokens == null)
						GLib.stdout.putc('\n');
					for (unowned var tok_ptr = tokens; tok_ptr != null; tok_ptr = tok_ptr.next) {
						corrected_something = true;

						var token = tok_ptr.data;
						var word = token.word;
						size_t pos = token.pos;

						if (mode == Mode.A)
							do_mode_a(dict, word, pos, line_count, terse_mode);
						else if (mode == Mode.L)
							do_mode_l(dict, word, line_count);
					}
				}
			}

			if (mode == Mode.A && corrected_something)
				GLib.stdout.putc('\n');
			GLib.stdout.flush();
		}

		return true;
	}

	public static int main(string[] args) {
		/* Initialize system locale */
		Intl.setlocale();
		get_charset(out charset);

		var ctx = new OptionContext("\n\nCheck spelling non-interactively.");
		ctx.set_help_enabled(true);
		ctx.add_main_entries(main_options, null);
		try {
			ctx.parse(ref args);
		} catch (OptionError e) {
			printerr("error %s\n", e.message);
			usage(ctx);
		}

		if (version) {
			print_version(GLib.stdout);
			exit(0);
		}

		/* Exit with usage if no mode is set. */
		if (mode == Mode.NONE)
			usage(ctx);

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
