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

void print_help(string prog) {
	GLib.stderr.printf(
		"""Usage: %s -a|-l|-h|-v [-L] [-d DICTIONARY] [FILE]
  -d DICTIONARY  use the given dictionary
  -p FILE        use the given personal word list
  -a             communicate non-interactively through a pipe like Ispell
  -l             list only the misspellings
  -L             display line numbers
  -h             display help and exit
  -v             display version information and exit
""", prog);
}

string get_line(FileStream fin) {
	string str = fin.read_line();

	if (str.length > 0) {
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
		GLib.stdout.write(native.data, bytes_written);
	} catch (GLib.ConvertError e) {
		/* Assume that it's already utf8 and glib is just being stupid. */
		print("%s", str);
	}
}

bool check_word(Dict dict, string word) {
	return word.length <= MIN_WORD_LENGTH ||
		dict.check(word, word.length) == 0;
}

void do_mode_a(Dict dict, string word, size_t start_pos, size_t lineCount, bool terse_mode) {
	if (check_word(dict, word)) {
		if (!terse_mode) {
			if (lineCount > 0)
				print("* %zu\n", lineCount);
			else
				print("*\n");
		}
	} else {
		string[] suggs = dict.suggest(word, word.length);
		if (suggs == null || suggs.length == 0) {
			print("# ");
			if (lineCount > 0)
				print("%zu ", lineCount);
			print_utf(word);
			print(" %zu\n", start_pos);
		} else {
			print("& ");
			if (lineCount > 0)
				print("%zu ", lineCount);
			print_utf (word);
			print(" %zu %zu:", suggs.length, start_pos);

			for (size_t i = 0; i < suggs.length; i++) {
				GLib.stdout.putc(' ');
				print_utf(suggs[i]);

				if (i != suggs.length - 1)
					GLib.stdout.putc(',');
			}
			GLib.stdout.putc('\n');

			dict.free_string_list(suggs);
		}
	}
}

void do_mode_l(Dict dict, string word, size_t lineCount) {
	if (!check_word(dict, word)) {
		if (lineCount > 0)
			print("%zu ", lineCount);
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
	long start_pos = 0;
	unowned string utf = line;

	while (utf[0] != '\0') {
		unichar uc;

		/* Skip non-word characters. */
		for (uc = utf.get_char();
			 uc != 0
				 && !dict.is_word_character(uc, WordPosition.START);
			 uc = utf.get_char()) {
			utf = utf.next_char();
		}
		start_pos = line.pointer_to_offset(utf);

		/* Skip over word characters. */
		for (;
			 uc != 0 &&
				 dict.is_word_character(uc, WordPosition.MIDDLE);
			 uc = utf.get_char()) {
			utf = utf.next_char();
		}

		/* Skip backwards over any characters that can't appear at the end of a word. */
		for (;
			 !dict.is_word_character(uc, WordPosition.END);
			 utf = utf.prev_char()) {
			uc = utf.get_char();
		}

		/* Save (word, position) tuple. */
		long cur_pos = line.pointer_to_offset(utf);
		if (cur_pos > start_pos) {
			var word = line.substring(start_pos, cur_pos);
			tokens.append(new Token(word, start_pos));
		}
	}

	return tokens;
}

errordomain Spelling {
	EMPTY_WORD,
    SYNTAX_ERROR,
}

int parse_file(FileStream fin, Mode mode, bool countLines, string? dictionary, string? perslist) {
	var terse_mode = false;

	if (mode == Mode.A)
		print_version(GLib.stdout);

	string lang;
	if (dictionary != null)
		lang = dictionary;
	else {
		lang = enchant_get_user_language();
		if (lang == null)
			return 1;
		if (lang == "C")
			lang = "en";
	}

	/* Enchant will get rid of trailing information like de_DE@euro or de_DE.ISO-8859-15 */

	var broker = new Broker();
	unowned var dict = broker.request_dict_with_pwl(lang, perslist);

	if (dict == null) {
		GLib.stderr.printf("No dictionary available for '%s'", lang);
		string errmsg = broker.get_error();
		if (errmsg != null)
			GLib.stderr.printf(": %s", errmsg);
		GLib.stderr.putc('\n');

		return 1;
	}

	var corrected_something = false;
	size_t lineCount = 0;
	for (var was_last_line = false; !was_last_line;) {
		bool mode_A_no_command = false;
		var str = get_line(fin);

		if (countLines)
			lineCount++;

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

					var token = tokens.data;
					var word = token.word;
					size_t pos = token.pos;

					if (mode == Mode.A)
						do_mode_a(dict, word, pos, lineCount, terse_mode);
					else if (mode == Mode.L)
						do_mode_l(dict, word, lineCount);
				}
			}
		}

		if (mode == Mode.A && corrected_something)
			GLib.stdout.putc('\n');
		GLib.stdout.flush();
	}

	return 0;
}

void usage(string progname) {
	print_help(progname);
	exit(1);
}

public int main(string[] args) {
	var mode = Mode.NONE;
	string file = null;
	var countLines = false;
	string dictionary = null;  /* -d dictionary */
	string perslist = null ; /* -p personal_word_list */

	/* Initialize system locale */
	Intl.setlocale();

	get_charset(out charset);
// FIXME
// #ifdef _WIN32
// 	/* If reading from stdin, its CP may not be the system CP (which glib's locale gives us) */
// 	if (GetFileType(GetStdHandle(STD_INPUT_HANDLE)) == FILE_TYPE_CHAR)
// 		charset = g_strdup_printf("CP%u", GetConsoleCP());
// #endif

	int optchar;
	while ((optchar = getopt(args, ":d:p:alvLmBh")) != -1) {
		switch (optchar) {
		case 'd':
			dictionary = optarg;  /* Emacs calls ispell with '-d dictionary'. */
			break;
		case 'p':
			perslist = optarg;
			break;
		/* The first mode specified takes precedence. */
		case 'a':
			if (mode == Mode.NONE)
				mode = Mode.A;
			break;
		case 'l':
			if (mode == Mode.NONE)
				mode = Mode.L;
			break;
		case 'L':
			countLines = true;
			break;
		case 'v':
			print_version(GLib.stdout);
			exit(0);
			break;
		case 'm':
		case 'B':
			/* Ignore: Emacs calls ispell with '-m' and '-B'. */
			break;
		case 'h':
			print_help(args[0]);
			exit(0);
			break;
		case ':':
			GLib.stderr.printf("missing argument to option\n");
			break;
		case '?':
			usage(args[0]);
			break;
		}
	}

	/* Get file argument if given. */
	if (optind < args.length)
		file = args[optind++];

	/* Exit with usage if either no mode is set, or if there are excess
	   non-option arguments. */
	if (mode == Mode.NONE || optind < args.length)
		usage(args[0]);

	/* Process the file or standard input. */
	FileStream fp = null;
	if (file != null) {
		fp = FileStream.open(file, "rb");
		if (fp == null) {
			GLib.stderr.printf("Error: Could not open the file \"%s\" for reading.\n", file);
			exit(1);
		}
	}
	return parse_file(fp == null ? GLib.stdin : fp, mode, countLines, dictionary, perslist);
}
