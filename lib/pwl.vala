#! /usr/bin/env -S vala --vapidir lib --pkg internal --pkg posix --pkg gnu util.vala
/* libenchant: Personal word lists
 * Copyright (C) 2003, 2004 Dom Lachowicz
 * Copyright (C) 2016-2025 Reuben Thomas <rrt@sc3d.org>
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
 * non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

using Posix;
using Gnu;

/**
 *  This file implements personal word list (PWL) dictionaries in the
 *  type EnchantPWL.
 */

// Utility constants and functions

const unichar BOM = 0xfeff;

void lock_file(FileStream f) {
	flock(f.fileno(), FlockOperation.EX);
}

void unlock_file(FileStream f) {
	flock(f.fileno(), FlockOperation.UN);
}

string utf8_strtitle(string str) {
	string upper_str = str.up(); /* for locale-sensitive casing */
	string lower_tail = upper_str.next_char().down();
	unichar title_case_char = upper_str.get_char(0).totitle();
	return "%s%s".printf(title_case_char.to_string(), lower_tail);
}

bool is_all_caps(string word) {
	bool has_cap = false;
	unichar c;
	for (int i = 0; word.get_next_char(ref i, out c); ) {
		UnicodeType type = c.type();
		switch(type) {
		case UnicodeType.UPPERCASE_LETTER:
			has_cap = true;
			break;
		case UnicodeType.TITLECASE_LETTER:
		case UnicodeType.LOWERCASE_LETTER:
			return false;

		case UnicodeType.CONTROL:
		case UnicodeType.FORMAT:
		case UnicodeType.UNASSIGNED:
		case UnicodeType.PRIVATE_USE:
		case UnicodeType.SURROGATE:
		case UnicodeType.MODIFIER_LETTER:
		case UnicodeType.OTHER_LETTER:
		case UnicodeType.SPACING_MARK:
		case UnicodeType.ENCLOSING_MARK:
		case UnicodeType.NON_SPACING_MARK:
		case UnicodeType.DECIMAL_NUMBER:
		case UnicodeType.LETTER_NUMBER:
		case UnicodeType.OTHER_NUMBER:
		case UnicodeType.CONNECT_PUNCTUATION:
		case UnicodeType.DASH_PUNCTUATION:
		case UnicodeType.CLOSE_PUNCTUATION:
		case UnicodeType.FINAL_PUNCTUATION:
		case UnicodeType.INITIAL_PUNCTUATION:
		case UnicodeType.OTHER_PUNCTUATION:
		case UnicodeType.OPEN_PUNCTUATION:
		case UnicodeType.CURRENCY_SYMBOL:
		case UnicodeType.MODIFIER_SYMBOL:
		case UnicodeType.MATH_SYMBOL:
		case UnicodeType.OTHER_SYMBOL:
		case UnicodeType.LINE_SEPARATOR:
		case UnicodeType.PARAGRAPH_SEPARATOR:
		case UnicodeType.SPACE_SEPARATOR:
		default:
			break;
		}
	}

	return has_cap;
}

bool is_title_case(string word) {
	int i = 0;
	unichar c;
	word.get_next_char(ref i, out c);
	UnicodeType type = c.type();
	if ((type != UnicodeType.UPPERCASE_LETTER && type != UnicodeType.TITLECASE_LETTER) ||
		c != c.totitle())
		return false;

	while (word.get_next_char(ref i, out c)) {
		type = c.type();
		if (type == UnicodeType.UPPERCASE_LETTER || type == UnicodeType.TITLECASE_LETTER)
			return false;
	}

	return true;
}

public class EnchantPWL {
	private string? filename;
	private time_t file_changed = 0;
	private HashTable<string, string> words = new HashTable<string, string>(str_hash, str_equal);

	public EnchantPWL(string? filename) {
		this.filename = filename;
	}

	void add_to_table(string word) {
		string normalized_word = word.normalize();
		if (!this.words.contains(normalized_word))
			this.words.insert(normalized_word, word);
	}

	public void add(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);

		this.refresh_from_file();
		this.add_to_table(word);

		if (this.filename != null) {
			FileStream? f = FileStream.open(this.filename, "a+");
			if (f != null) {
				/* Since this method does not signal I/O errors, only use
				   return values to avoid doing things that seem futile. */
				lock_file(f);
				Posix.Stat stats;
				if (Posix.stat(this.filename, out stats) == 0)
					this.file_changed = stats.st_mtime;

				/* Add a newline if the file doesn't end with one. */
				if (f.seek(-1, FileSeek.END) == 0) {
					int c = f.getc();
					f.seek(0, FileSeek.CUR); /* ISO C requires positioning between read and write. */
					if (c != '\n')
						f.putc('\n');
				}

				if (f.puts(word) != FileStream.EOF)
					f.putc('\n');
				unlock_file(f);
			}
		}
	}

	public void remove(string word_buf, real_ssize_t len) {
		// 'check' calls 'refresh_from_file' for us.
		if (this.check(word_buf, len) > 0)
			return;

		string word = buf_to_utf8_string(word_buf, len);
		this.words.remove(word.normalize());

		if (this.filename != null) {
			string contents;
			size_t length;
			try {
				FileUtils.get_contents(this.filename, out contents, out length);
			} catch (GLib.FileError e) {
				return;
			}

			var f = FileStream.open(this.filename, "wb"); /* binary because get_contents reads binary */
			if (f != null) {
				lock_file(f);
				long filestart = 0;

				// Copy BOM if present.
				if (BOM == contents.get_char(filestart)) {
					contents = contents.next_char();
					f.puts(BOM.to_string());
				}

				for (int start_index = 0;;) {
					// find word
					var word_pos = contents.index_of(word, start_index);
					if (word_pos == -1) {
						f.puts(contents.substring(start_index));
						break;
					} else {
						if ((word_pos == 0 || contents[word_pos - 1] == '\n' || contents[word_pos - 1] == '\r') &&
							(word_pos == contents.length || contents[word_pos + word.length] == '\n' || contents[word_pos + word.length] == '\r')) {
							f.puts(contents.substring(start_index, word_pos - start_index));
							start_index = word_pos + word.length;
							while (contents[start_index] == '\n' || contents[start_index] == '\r')
								++start_index;
						} else {
							f.puts(contents.substring(start_index, word_pos - start_index + 1));
							start_index = word_pos + 1;
						}
					}
				}

				Posix.Stat stats;
				if (Posix.stat(this.filename, out stats) == 0)
					this.file_changed = stats.st_mtime;

				unlock_file(f);
			}
		}
	}

	public int check(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);
		this.refresh_from_file();

		if (this.words.contains(word.normalize()))
			return 0;

		bool all_caps = false;
		if (is_title_case(word) || (all_caps = is_all_caps(word))) {
			string lower_case_word = word.down();
			if (this.words.contains(lower_case_word.normalize()))
				return 0;

			if (all_caps) {
				string title_case_word = utf8_strtitle(word);
				if (this.words.contains(title_case_word.normalize()))
					return 0;
			}
		}

		return 1; /* not found */
	}

	void refresh_from_file() {
		if (this.filename == null)
			return;

		Posix.Stat stats;
		if (Posix.stat(this.filename, out stats) == -1)
			return; /* presumably won't be able to open the file either */
		if (this.file_changed == stats.st_mtime) /* nothing changed since last read */
			return;

		this.words = new HashTable<string, string>(str_hash, str_equal);

		FileStream? f = FileStream.open(this.filename, "r");
		if (f == null)
			return;

		this.file_changed = stats.st_mtime;
		lock_file(f);

		size_t line_number = 1;
		string line;
		for (; (line = f.read_line()) != null; ++line_number) {
			if (line_number == 1 && BOM == line.get_char())
				line = line.next_char();

			line = line.chomp();
			if (line[0] != '\0' && line[0] != '#') {
				if (line.validate())
					this.add_to_table(line);
				else
					warning("Bad UTF-8 sequence in %s at line:%zu", this.filename, line_number);
			}
		}

		unlock_file(f);
	}
}

int check_impl(EnchantProviderDict me, string word, real_size_t len) {
	return 1;
}

[CCode (array_length_type = "size_t")]
string[]? suggest_impl(EnchantProviderDict me, string word, real_size_t len) {
	return null;
}

public class EnchantPwlDict : EnchantProviderDict {
	public EnchantPwlDict() {
		base(null, "Personal Wordlist");
		this.check_method = check_impl;
		this.suggest_method = suggest_impl;
	}
}
