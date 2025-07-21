#! /usr/bin/env -S vala --vapidir src --vapidir lib --pkg internal provider.vala session.vala
/* enchant: ProviderDict
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

public class EnchantProviderDict {
	public void *user_data;
	public EnchantProvider? provider;
	public string language_tag;
	public string error;

	public EnchantProviderDict(EnchantProvider? provider, string tag) {
		this.provider = provider;
		this.language_tag = tag;
		this.error = null;
	}

	~EnchantProviderDict() {
		if (this.provider != null)
			this.provider.dispose_dict(this.provider, this);
	}

	public void set_error(string err) {
		debug("enchant_provider_dict_set_error: %s", err);
		this.error = err;
	}

	public virtual int check(string word, real_size_t len) {
		return 1;
	}

	/* Returns an array of UTF-8 encoded strings. Elements
	   of word may be NULL. */
	[CCode (array_length_pos = 2, array_length_type = "size_t")]
	public virtual string[]? suggest(string word_buf, real_size_t len) {
		return null;
	}

	public virtual void add_to_session(string word, real_size_t len) {}

	public virtual void remove_from_session(string word, real_size_t len) {}

	public virtual unowned string get_extra_word_characters() {
		return "";
	}

	public virtual int is_word_character(uint32 uc_in, real_size_t n) {
		return -1;
	}
}
