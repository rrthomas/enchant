#! /usr/bin/env -S vala --vapidir src --vapidir lib --pkg internal --pkg gnu broker.vala dict.vala
/* libenchant: Composite dictionaries
 * Copyright (C) 2024-2025 Reuben Thomas <rrt@sc3d.org>
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

public class EnchantCompositeDict : EnchantProviderDict {
	unowned EnchantBroker broker;
	public SList<weak EnchantDict> session_list;

	public EnchantCompositeDict(EnchantBroker broker, owned SList<weak EnchantDict> session_list, string tag) {
		base(null, tag);
		this.broker = broker;
		this.session_list = (owned)session_list;
		this.check_method = composite_dict_check;
		this.suggest_method = composite_dict_suggest;
		this.add_to_session_method = composite_dict_add_to_session;
		this.remove_from_session_method = composite_dict_remove_from_session;
	}
}

int composite_dict_check(EnchantProviderDict? self, string word_buf, real_size_t len) {
	if (self == null || word_buf == null)
		return -1;
	string word = buf_to_utf8_string(word_buf, (real_ssize_t)len);
	if (word == null)
		return -1;

	// Check word in all dictionaries.
	// Signal error (-1) if and only if all dictionaries error.
	var cdict = (EnchantCompositeDict)(self);
	int err = -1;
	foreach (EnchantDict session in cdict.session_list) {
		int found = EnchantDict.check(session, word, (real_ssize_t)len);
		if (found == 0)
			return 0;
		if (found == 1)
			err = 1;
	}
	return err;
}

[CCode (array_length_pos = 4, array_length_type = "size_t")]
string[]? composite_dict_suggest(EnchantProviderDict me, string word, real_size_t len) {
	var cdict = (EnchantCompositeDict)(me);
	var error = true;
	var res = new Array<string>();
	foreach (EnchantDict dict in cdict.session_list) {
		var suggs = dict.suggest(word, (real_ssize_t)len);
		if (suggs != null) {
			error = false;
			if (suggs.length > 0)
				for (real_size_t i = 0; i < suggs.length; i++)
					res.append_val(suggs[i]);
		}
	}
	if (error == true)
		return null;
	return res.steal();
}

void composite_dict_add_to_session(EnchantProviderDict me, string word, real_size_t len) {
	var cdict = (EnchantCompositeDict)(me);
	assert(cdict.session_list.length() > 0);
	cdict.session_list.data.add_to_session(word, (real_ssize_t)len);
}

void composite_dict_remove_from_session(EnchantProviderDict me, string word, real_size_t len) {
	var cdict = (EnchantCompositeDict)(me);
	assert(cdict.session_list.length() > 0);
	cdict.session_list.data.remove_from_session(word, (real_ssize_t)len);
}
