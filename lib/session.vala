#! /usr/bin/env -S vala --vapidir lib --pkg internal pwl.vala
/* libenchant: Session
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

public class EnchantSession {
	public GenericSet<string> session_include;
	public GenericSet<string> session_exclude;
	public EnchantPWL pwl;
	public EnchantPWL exclude_pwl;

	public string personal_filename;
	public string exclude_filename;
	public string language_tag;

	public string error;
	public EnchantProvider provider;

	EnchantSession() {
		this.session_include = new GenericSet<string>(str_hash, str_equal);
		this.session_exclude = new GenericSet<string>(str_hash, str_equal);
	}

	public static EnchantSession? with_implicit_pwl(EnchantProvider? provider, string lang, string? pwl) {
		string user_config_dir = enchant_get_user_config_dir();
		if (user_config_dir == null || lang == null)
			return null;

		DirUtils.create_with_parents(user_config_dir, 0700);
		if (pwl == null)
			return EnchantSession.with_pwl(
				provider,
				Path.build_filename(user_config_dir, "%s.dic".printf(lang)),
				Path.build_filename(user_config_dir, "%s.exc".printf(lang)),
				lang);

		return EnchantSession.with_pwl(provider, pwl, null, lang);
	}

	public static EnchantSession? with_pwl(
		EnchantProvider? provider,
		string? pwlname,
		string? exclname,
		string lang
		) {
		EnchantPWL pwl = new EnchantPWL(pwlname);
		EnchantPWL exclude_pwl = new EnchantPWL(exclname);

		EnchantSession session = new EnchantSession();
		session.pwl = (owned)pwl;
		session.exclude_pwl = (owned)exclude_pwl;
		session.provider = provider;
		session.language_tag = lang;
		session.personal_filename = pwlname;
		session.exclude_filename = exclname;

		return session;
	}

	public void add(string word) {
		this.session_exclude.remove(word);
		this.session_include.add(word);
	}

	public void remove(string word) {
		this.session_include.remove(word);
		this.session_exclude.add(word);
	}

	public bool exclude(string word) {
		return !this.session_include.contains(word) &&
			   (this.session_exclude.contains(word) ||
				this.exclude_pwl.check(word, word.length) == 0);
	}

	public bool contains(string word) {
		return this.session_include.contains(word) ||
			   (this.pwl.check(word, word.length) == 0 &&
				this.exclude_pwl.check(word, word.length) != 0);
	}

	public void clear_error() {
		this.error = null;
	}
}
