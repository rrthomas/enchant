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

public class EnchantDict {
	public void *user_data;
	public EnchantSession session;

	~EnchantDict() {
		unowned EnchantProvider owner = this.session.provider;
		if (owner != null)
			owner.dispose_dict(owner, this);
	}

	/* returns utf8*/
	[CCode (array_length_type = "size_t")]
	public string[]? DictSuggest(string word, real_size_t len) {
		return null;
	}

	public virtual unowned string get_extra_word_characters() {
		return "";
	}

	public virtual int is_word_character(uint32 uc_in, real_size_t n)
	requires(n <= 2)
	{
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

	public virtual int check(string? word_buf, real_ssize_t len) {
		if (this== null || word_buf == null)
			return -1;
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return -1;

		this.session.clear_error();

		/* first, see if it's to be excluded*/
		if (this.session.exclude(word))
			return 1;

		/* then, see if it's in our pwl or session*/
		if (this.session.contains(word))
			return 0;

		return 1;
	}

	/* Filter out suggestions that are null, invalid UTF-8 or in the exclude
	   list.  Returns a null-terminated array. */
	string[]? filter_suggestions(string[] suggs) {
		var sb = new StrvBuilder();
		foreach (string sugg in suggs) {
			if (sugg == null) {
				this.session.error = @"null entry in suggestions returned by $(this.session.provider.identify(this.session.provider))";
				return null;
			}
			if (sugg.validate() && !this.session.exclude(sugg))
				sb.add(sugg);
		}

		return sb.end();
	}

	[CCode (array_length_pos = 2, array_length_type = "size_t")]
	public virtual string[]? suggest(string word_buf, real_ssize_t len) {
		return null;
	}

	public void add(string word_buf, real_ssize_t len) {
		this.add_to_session(word_buf, len);
		session.pwl.add(word_buf, len);
		session.exclude_pwl.remove(word_buf, len);
	}

	public virtual void add_to_session(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return;
		this.session.clear_error();
		this.session.add(word);
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

	public virtual void remove_from_session(string word_buf, real_ssize_t len) {
		string word = buf_to_utf8_string(word_buf, len);
		if (word == null)
			return;
		this.session.clear_error();
		this.session.remove(word);
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
