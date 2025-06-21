#! /usr/bin/env -S vala --vapidir lib --pkg gnu
/* enchant: Provider
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

using Gnu;

string? enchant_get_user_config_dir() {
	unowned string env = Environment.get_variable("ENCHANT_CONFIG_DIR");
	if (env != null)
		try {
			return Filename.to_utf8(env, -1, null, null);
		} catch (ConvertError e) {
			return null;
		}
	return Path.build_filename(Environment.get_user_config_dir(), "enchant");
}

SList enchant_get_conf_dirs() {
	var conf_dirs = new SList<string>();

	string? pkgdatadir = relocate("%s-%s".printf(PKGDATADIR, ENCHANT_MAJOR_VERSION));
	if (pkgdatadir != null)
		conf_dirs.append(pkgdatadir);

	string? sysconfdir = relocate(SYSCONFDIR);
	if (sysconfdir != null) {
		string pkgconfdir = Path.build_filename(sysconfdir, "enchant-%s".printf(ENCHANT_MAJOR_VERSION));
		if (pkgconfdir != null)
			conf_dirs.append(pkgconfdir);
	}

	string? user_config_dir = enchant_get_user_config_dir();
	if (user_config_dir != null)
		conf_dirs.append(user_config_dir);

	return conf_dirs;
}

public string enchant_get_prefix_dir() {
	return relocate(INSTALLPREFIX);
}

[CCode (has_target = false)]
public delegate void ProviderDispose(EnchantProvider me);
[CCode (has_target = false)]
public delegate EnchantDict ProviderRequestDict(EnchantProvider me, string tag);
[CCode (has_target = false)]
public delegate void ProviderDisposeDict(EnchantProvider me, EnchantDict dict);
[CCode (has_target = false)]
public delegate int ProviderDictionaryExists(EnchantProvider me, string tag);
/* returns utf8*/
[CCode (has_target = false)]
public delegate unowned string ProviderIdentify(EnchantProvider me);
/* returns utf8*/
[CCode (has_target = false)]
public delegate unowned string ProviderDescribe(EnchantProvider me);
[CCode (has_target = false, array_length_type = "size_t")]
public delegate string[] ProviderListDicts(EnchantProvider me);

public class EnchantProvider {
	public void *user_data;
	public Module module;
	public unowned EnchantBroker? owner;
	public ProviderDispose dispose;
	public ProviderRequestDict request_dict;
	public ProviderDisposeDict dispose_dict;
	public ProviderDictionaryExists dictionary_exists;
	public ProviderIdentify identify;
	public ProviderDescribe describe;
	public ProviderListDicts list_dicts;

	~EnchantProvider() {
		var module = (owned)this.module;
		if (this.dispose != null)
			this.dispose(this);
		/* close module only after invoking dispose */
		module.close();
	}

	public static bool is_valid(EnchantProvider provider) {
		if (provider == null)
			warning("EnchantProvider cannot be NULL");
		else if (provider.request_dict == null)
			warning("EnchantProvider's request_dict method cannot be NULL");
		else if (provider.dispose_dict == null)
			warning("EnchantProvider's dispose_dict method cannot be NULL");
		else if (provider.identify == null)
			warning("EnchantProvider's identify method cannot be NULL");
		else if (!provider.identify(provider).validate())
			warning("EnchantProvider's identify method does not return valid UTF-8");
		else if (provider.describe == null)
			warning("EnchantProvider's describe method cannot be NULL");
		else if (!provider.describe(provider).validate())
			warning("EnchantProvider's describe method does not return valid UTF-8");
		else if (provider.list_dicts == null)
			warning("EnchantProvider's list_dicts method cannot be NULL");
		else
			return true;

		return false;
	}

	public void set_error(string err) {
		unowned EnchantBroker broker = this.owner;
		if (broker == null)
			return;

		debug("enchant_provider_set_error: %s", err);
		broker.error = err;
	}

	public int _dictionary_exists(string tag) {
		if (this.dictionary_exists != null)
			return this.dictionary_exists(this, tag);

		foreach (string dict in this.list_dicts(this))
			if (dict == tag)
				return 1;
		return 0;
	}

	public string get_user_dict_dir() {
		string config_dir = enchant_get_user_config_dir();
		return Path.build_filename(config_dir, this.identify(this));
	}
}
