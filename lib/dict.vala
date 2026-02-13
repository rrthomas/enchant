#! /usr/bin/env -S vala --vapidir lib --pkg internal pwl.vala
/* libenchant: Dict
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

public class EnchantDict {
	public GenericSet<string> session_include;
	public GenericSet<string> session_exclude;
	public EnchantPWL pwl;
	public EnchantPWL exclude_pwl;
	EnchantProviderDict dict;

	public string personal_filename;
	public string exclude_filename;

	EnchantDict() {
		this.session_include = new GenericSet<string>(str_hash, str_equal);
		this.session_exclude = new GenericSet<string>(str_hash, str_equal);
	}

	public static EnchantDict with_implicit_pwl(EnchantProviderDict dict, string lang, string? pwl) {
		if (pwl != null)
			return EnchantDict.with_pwl(dict, pwl, null);

		string user_config_dir = enchant_get_user_config_dir();
		DirUtils.create_with_parents(user_config_dir, 0700);
		return EnchantDict.with_pwl(
			dict,
			Path.build_filename(user_config_dir, "%s.dic".printf(lang)),
			Path.build_filename(user_config_dir, "%s.exc".printf(lang)));
	}

	public static EnchantDict with_pwl(EnchantProviderDict dict, string pwlname, string? exclname) {
		EnchantPWL pwl = new EnchantPWL(pwlname);
		EnchantPWL exclude_pwl = new EnchantPWL(exclname, true);

		EnchantDict session = new EnchantDict();
		session.dict = dict;
		session.pwl = (owned)pwl;
		session.exclude_pwl = (owned)exclude_pwl;
		session.personal_filename = pwlname;
		session.exclude_filename = exclname;

		return session;
	}

	bool excluded(string word) {
		return !this.session_include.contains(word) &&
			(this.session_exclude.contains(word) ||
			 this.exclude_pwl.check(this, word, word.length) == 0);
	}

	bool contains(string word) {
		return this.session_include.contains(word) ||
			(this.pwl.check(this, word, word.length) == 0 &&
			 this.exclude_pwl.check(this, word, word.length) != 0);
	}

	public unowned string get_extra_word_characters() {
		return dict.get_extra_word_characters_method != null ?
			   dict.get_extra_word_characters_method(dict) : "";
	}

	public static int is_word_character(EnchantDict? self, uint32 uc_in, real_size_t n)
	requires(n <= 2)
	{
		if (self != null && self.dict.is_word_character_method != null)
			return self.dict.is_word_character_method(self.dict, uc_in, n);

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

	/* This is a static method method because Vala does not let us
	 * alter the value returned when an argument is invalid.
	 * In this case, we want to return -1 when 'dict' is null. */
	public static int check(EnchantDict? self, string? word_buf, real_ssize_t len) {
		if (self == null || word_buf == null)
			return -1;
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return -1;

		self.clear_error();

		/* first, see if it's excluded */
		if (self.excluded(word))
			return 1;

		/* then, see if it's in our pwl or session */
		if (self.contains(word))
			return 0;

		return self.dict.check_method(self.dict, word, word.length);
	}

	/* Filter out suggestions that are null, invalid UTF-8 or in the exclude
	   list.  Returns a null-terminated array. */
	string[]? filter_suggestions(string[] suggs) {
		var sb = new StrvBuilder();
		foreach (string sugg in suggs)
			if (sugg != null && sugg.validate() && !this.excluded(sugg))
				sb.add(sugg);
		return sb.end();
	}

	[CCode (array_length_pos = 3, array_length_type = "size_t")]
	public string[]? suggest(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return null;

		this.clear_error();

		/* Check for suggestions from provider dictionary */
		string[]? dict_suggs = dict.suggest_method(dict, word, word.length);
		if (dict_suggs != null)
			dict_suggs = this.filter_suggestions(dict_suggs);

		return dict_suggs;
	}

	public void add(string word_buf, real_ssize_t len) {
		this.pwl.add(this, word_buf, len);
		this.exclude_pwl.remove(this, word_buf, len);
		this.add_to_session(word_buf, len);
	}

	public void add_to_session(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return;
		this.clear_error();
		this.session_exclude.remove(word);
		this.session_include.add(word);
		if (dict.add_to_session_method != null)
			dict.add_to_session_method(dict, word, word.length);
	}

	public int is_added(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return 0;
		this.clear_error();
		return this.contains(word) ? 1 : 0;
	}

	public void remove(string word_buf, real_ssize_t len) {
		this.pwl.remove(this, word_buf, len);
		this.exclude_pwl.add(this, word_buf, len);
		this.remove_from_session(word_buf, len);
	}

	public void remove_from_session(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return;
		this.clear_error();
		this.session_include.remove(word);
		this.session_exclude.add(word);
		if (dict.remove_from_session_method != null)
			dict.remove_from_session_method(dict, word, word.length);
	}

	public int is_removed(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return 0;
		this.clear_error();
		return this.excluded(word) ? 1 : 0;
	}

	/* Stub for obsolete API. */
	public void store_replacement(string mis, real_ssize_t mis_len,
								  string cor, real_ssize_t cor_len) { }

	public void free_string_list(char **string_list) {
		this.clear_error();
		strfreev((string[])(owned)string_list);
	}

	public void describe(EnchantDictDescribeFn fn, void *user_data)
	requires(fn != null)
	{
		this.clear_error();

		string name;
		string desc;
		string file;
		if (dict.provider != null) {
			file = dict.provider.module.name();
			name = dict.provider.identify(dict.provider);
			desc = dict.provider.describe(dict.provider);
		} else {
			file = this.personal_filename;
			name = "Personal Wordlist";
			desc = "Personal Wordlist";
		}

		fn(dict.language_tag, name, desc, file, user_data);
	}

	// FIXME: This API is only used for testing.
	public void set_error(string err) {
		this.dict.set_error(err);
	}

	public unowned string get_error() {
		return this.dict.error;
	}

	public void clear_error() {
		if (this.dict != null) {
			this.dict.error = null;
		}
	}
}
