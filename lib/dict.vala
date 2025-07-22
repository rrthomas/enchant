#! /usr/bin/env -S vala --vapidir src --vapidir lib --pkg internal provider.vala session.vala
/* enchant: Dict
 * Copyright (C) 2003, 2004 Dom Lachowicz
 * Copyright (C) 2016-2024 Reuben Thomas <rrt@sc3d.org>
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

[CCode (has_target = false)]
public delegate int DictCheck(EnchantDict me, string word, real_size_t len);
/* returns utf8*/
[CCode (has_target = false, array_length_type = "size_t")]
public delegate string[]? DictSuggest(EnchantDict me, string word, real_size_t len);
[CCode (has_target = false)]
public delegate void DictAddToSession(EnchantDict me, string word, real_size_t len);
[CCode (has_target = false)]
public delegate void DictRemoveFromSession(EnchantDict me, string word, real_size_t len);
[CCode (has_target = false)]
public delegate unowned string DictGetExtraWordCharacters(EnchantDict me);
[CCode (has_target = false)]
public delegate int DictIsWordCharacter(EnchantDict me, uint32 uc_in, real_size_t n);

public class EnchantDict {
	public void *user_data;
	public EnchantSession session;

	// Provider methods
	public DictCheck check_method;
	public DictSuggest suggest_method;
	public DictAddToSession? add_to_session_method;
	public DictRemoveFromSession? remove_from_session_method;
	public DictGetExtraWordCharacters? get_extra_word_characters_method;
	public DictIsWordCharacter? is_word_character_method;

	~EnchantDict() {
		unowned EnchantProvider owner = this.session.provider;
		if (owner != null)
			owner.dispose_dict(owner, this);
	}

	public unowned string get_extra_word_characters() {
		return this.get_extra_word_characters_method != null ?
			   this.get_extra_word_characters_method(this) : "";
	}

	public static int is_word_character(EnchantDict? self, uint32 uc_in, real_size_t n)
	requires(n <= 2)
	{
		if (self != null && self.is_word_character_method != null)
			return self.is_word_character_method(self, uc_in, n);

		unichar uc = (unichar)uc_in;

		/* Accept quote marks anywhere except at the end of a word */
		if (uc == "'".get_char() || uc == "’".get_char())
			return n < 2 ? 1 : 0;

		UnicodeType type = uc.type();

		switch (type) {
		case UnicodeType.MODIFIER_LETTER:
		case UnicodeType.LOWERCASE_LETTER:
		case UnicodeType.TITLECASE_LETTER:
		case UnicodeType.UPPERCASE_LETTER:
		case UnicodeType.OTHER_LETTER:
		case UnicodeType.SPACING_MARK:
		case UnicodeType.ENCLOSING_MARK:
		case UnicodeType.NON_SPACING_MARK:
		case UnicodeType.DECIMAL_NUMBER:
		case UnicodeType.LETTER_NUMBER:
		case UnicodeType.OTHER_NUMBER:
		case UnicodeType.CONNECT_PUNCTUATION:
			return 1; /* Enchant 1.3.0 defines word chars like this. */

		case UnicodeType.DASH_PUNCTUATION:
			if ((n == 1) && (type == UnicodeType.DASH_PUNCTUATION))
				return 1; /* hyphens only accepted within a word. */
			return 0;

		default:
			return 0;
		}
	}

	public void set_error(string err) {
		this.session.clear_error();
		debug("enchant_dict_set_error: %s", err);
		session.error = err;
	}

	public unowned string get_error() {
		return this.session.error;
	}

	/* This is a static method method because Vala does not let us
	 * alter the value returned when an argument is invalid.
	 * In this case, we want to return -1 when 'dict' is null. */
	public static int check(EnchantDict? self, string? word_buf, real_ssize_t len) {
		if (self == null || word_buf == null)
			return -1;
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return -1;

		self.session.clear_error();

		/* first, see if it's to be excluded*/
		if (self.session.exclude(word))
			return 1;

		/* then, see if it's in our pwl or session*/
		if (self.session.contains(word))
			return 0;

		return self.check_method(self, word, word.length);
	}

	/* Filter out suggestions that are null, invalid UTF-8 or in the exclude
	   list.  Returns a null-terminated array. */
	string[]? filter_suggestions(string[] suggs) {
		var sb = new StrvBuilder();
		foreach (string sugg in suggs)
			if (sugg != null && sugg.validate() && !this.session.exclude(sugg))
				sb.add(sugg);
		return sb.end();
	}

	[CCode (array_length_pos = 3, array_length_type = "size_t")]
	public string[]? suggest(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return null;

		this.session.clear_error();

		/* Check for suggestions from provider dictionary */
		string[]? dict_suggs = this.suggest_method(this, word, word.length);
		if (dict_suggs != null)
			dict_suggs = this.filter_suggestions(dict_suggs);

		return dict_suggs;
	}

	public void add(string word_buf, real_ssize_t len) {
		this.add_to_session(word_buf, len);
		session.pwl.add(word_buf, len);
		session.exclude_pwl.remove(word_buf, len);
	}

	public void add_to_session(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return;
		this.session.clear_error();
		this.session.add(word);
		if (this.add_to_session_method != null)
			this.add_to_session_method(this, word, word.length);
	}

	public int is_added(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return 0;
		this.session.clear_error();
		return session.contains(word) ? 1 : 0;
	}

	public void remove(string word_buf, real_ssize_t len) {
		this.remove_from_session(word_buf, len);
		session.pwl.remove(word_buf, len);
		session.exclude_pwl.add(word_buf, len);
	}

	public void remove_from_session(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return;
		this.session.clear_error();
		this.session.remove(word);
		if (this.remove_from_session_method != null)
			this.remove_from_session_method(this, word, word.length);
	}

	public int is_removed(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return 0;
		this.session.clear_error();
		return session.exclude(word) ? 1 : 0;
	}

	/* Stub for obsolete API. */
	public void store_replacement(string mis, real_ssize_t mis_len,
								  string cor, real_ssize_t cor_len) { }

	public void free_string_list(char **string_list) {
		this.session.clear_error();
		strfreev((string[])(owned)string_list);
	}

	public void describe(EnchantDictDescribeFn fn, void *user_data)
	requires(fn != null)
	{
		this.session.clear_error();
		unowned EnchantProvider provider = this.session.provider;

		string name;
		string desc;
		string file;
		if (provider != null) {
			file = provider.module.name();
			name = provider.identify(provider);
			desc = provider.describe(provider);
		} else {
			file = session.personal_filename;
			name = "Personal Wordlist";
			desc = "Personal Wordlist";
		}

		string tag = session.language_tag;
		fn(tag, name, desc, file, user_data);
	}
}
