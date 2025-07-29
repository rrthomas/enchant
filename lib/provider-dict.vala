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

[CCode (has_target = false)]
public delegate int DictCheck(EnchantProviderDict me, string word, real_size_t len);
/* returns utf8*/
[CCode (has_target = false, array_length_type = "size_t")]
public delegate string[]? DictSuggest(EnchantProviderDict me, string word, real_size_t len);
[CCode (has_target = false)]
public delegate void DictAddToSession(EnchantProviderDict me, string word, real_size_t len);
[CCode (has_target = false)]
public delegate void DictRemoveFromSession(EnchantProviderDict me, string word, real_size_t len);
[CCode (has_target = false)]
public delegate unowned string DictGetExtraWordCharacters(EnchantProviderDict me);
[CCode (has_target = false)]
public delegate int DictIsWordCharacter(EnchantProviderDict me, uint32 uc_in, real_size_t n);

public class EnchantProviderDict {
	public void *user_data;
	public EnchantProvider? provider;
	public string language_tag;
	public string error;

	// Provider methods
	public DictCheck check_method;
	public DictSuggest suggest_method;
	public DictAddToSession? add_to_session_method;
	public DictRemoveFromSession? remove_from_session_method;
	public DictGetExtraWordCharacters? get_extra_word_characters_method;
	public DictIsWordCharacter? is_word_character_method;

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
}
