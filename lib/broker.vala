/* libenchant: Broker
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

string normalize_dictionary_tag(string dict_tag) {
	string new_tag = dict_tag.strip();

	/* strip off "@euro" */
	new_tag = new_tag.substring(0, new_tag.index_of_char('@'));
	/* strip off ".UTF-8" */
	new_tag = new_tag.substring(0, new_tag.index_of_char('.'));

	/* everything before first '_' or '-' is converted to lower case */
	var sb = new StringBuilder();
	int i;
	for (i = 0;
		 new_tag[i] != '\0' && new_tag[i] != '-' && new_tag[i] != '_';
		 ++i)
		sb.append_c(new_tag[i].tolower());

	/* turn en-GB into en_GB */
	if (new_tag[i] == '-' || new_tag[i] == '_')
		sb.append_c('_');

	/* first run of alphanumberics after first '_' is converted to upper
	   case */
	if (new_tag[i] != '\0')
		for (++i; new_tag[i].isalnum(); ++i)
			sb.append_c(new_tag[i].toupper());

	/* Copy remaining characters */
	while (new_tag[i] != '\0')
		sb.append_c(new_tag[i++]);

	return sb.str;
}

string iso_639_from_tag(string dict_tag) {
	return dict_tag.substring(0, dict_tag.index_of_char('_'));
}

[CCode (has_target = false)]
delegate EnchantProvider EnchantProviderInitFunc();
[CCode (has_target = false)]
delegate void EnchantPreConfigureFunc(EnchantProvider provider, string module_dir);

[Compact (opaque = true)]
public class EnchantBroker {
	SList<EnchantProvider> provider_list;	/* list of all of the spelling backend providers */
	HashTable<string, string> provider_ordering; /* map of language tag -> provider order */
	GenericSet<EnchantDict> dicts;

	string _error;

	[CCode (cname = "enchant_broker_init")]
	public EnchantBroker() {
		if (!Module.supported())
			return;

		this.load_providers();
		this.load_provider_ordering();
		this.dicts = new GenericSet<EnchantDict>(direct_hash, direct_equal);
	}

	~EnchantBroker() {
		// Do not crash if called from C without an instance.
		// Use return_if_fail to skip Vala's deallocation code.
		return_if_fail(this != null);
	}

	public void clear_error() {
		this.error = null;
	}

	public string error {
		get { return _error;}
		set { _error = value; }
	}

	public void set_ordering(string tag, string ordering) {
		this.clear_error();

		string tag_dupl = normalize_dictionary_tag(tag);
		string ordering_dupl = ordering.strip();

		if (tag_dupl != null && tag_dupl.length > 0 &&
			ordering_dupl != null && ordering_dupl.length > 0)
			this.provider_ordering.insert(tag_dupl, ordering_dupl);
	}

	void load_providers() {
		string? module_dir = relocate("%s-%s".printf(PKGLIBDIR, ENCHANT_MAJOR_VERSION));
		if (module_dir != null)
			this.load_providers_in_dir(module_dir);
	}

	void load_providers_in_dir(string dir_name) {
		GLib.Dir dir;
		try {
			dir = GLib.Dir.open(dir_name, 0);
		} catch (GLib.FileError e) {
			return;
		}

		string dir_entry;
		size_t g_module_suffix_len = Module.SUFFIX.length;
		while ((dir_entry = dir.read_name()) != null) {
			Module? module = null;
			EnchantProvider? provider = null;

			size_t entry_len = dir_entry.length;
			if (entry_len > g_module_suffix_len &&
				dir_entry[0] != '.' && /* Skip hidden files */
				dir_entry.substring((long)(entry_len - g_module_suffix_len)) == Module.SUFFIX) {
				string filename = Path.build_filename(dir_name, dir_entry);
				module = Module.open(filename, 0);
				if (module != null) {
					void *init_func;
					if (module.symbol("init_enchant_provider", out init_func)
						&& init_func != null) {
						provider = ((EnchantProviderInitFunc)init_func)();
						if (!EnchantProvider.is_valid(provider)) {
							warning("Error loading plugin: %s's init_enchant_provider returned invalid provider", dir_entry);
							if (provider != null) {
								if (provider.dispose != null)
									provider.dispose(provider);
								provider = null;
							}
						}
					}
				} else
					warning("Error loading plugin: %s", Module.error());
			}
			if (provider != null) {
				/* optional entry point to allow modules to look for associated files */
				void *conf_func;
				if (module.symbol("configure_enchant_provider", out conf_func)
					&& conf_func != null) {
					((EnchantPreConfigureFunc)conf_func)(provider, dir_name);
					if (!EnchantProvider.is_valid(provider)) {
						warning("Error loading plugin: %s's configure_enchant_provider modified provider and it is now invalid", dir_entry);
						if (provider.dispose != null)
							provider.dispose(provider);
						provider = null;
					}
				}
			}
			if (provider != null) {
				provider.module = (owned)module;
				provider.owner = this;
				this.provider_list.append((owned)provider);
			}
		}
	}

	void load_ordering_from_file(string file) {
		IOChannel ch = null;
		try {
			ch = new IOChannel.file(file, "r");
		} catch (FileError e) {
			debug("could not open ordering file %s: %s", file, e.message);
			return;
		}
		GLib.assert(ch != null);

		debug("reading ordering file %s", file);
		string line;
		size_t terminator;
		try {
			while (IOStatus.NORMAL == ch.read_line(out line, null, out terminator)) {
				var colon = line.index_of_char(':');
				if (colon != -1) {
					string tag = line.substring(0, colon);
					string ordering = line.substring(colon + 1, (long)terminator - colon - 1);
					this.set_ordering(tag, ordering);
				}
			}
		} catch (ConvertError e) {
			return;
		} catch (IOChannelError e) {
			return;
		}
	}

	public void load_provider_ordering() {
		this.provider_ordering = new HashTable<string, string>(str_hash, str_equal);

		SList<string> conf_dirs = enchant_get_conf_dirs();
		foreach (string dir in conf_dirs) {
			string ordering_file = Path.build_filename(dir, "enchant.ordering");
			this.load_ordering_from_file(ordering_file);
		}
	}

	public SList<unowned EnchantProvider> get_ordered_providers(string tag) {
		string ordering = this.provider_ordering.lookup(tag);
		if (ordering == null)
			ordering = this.provider_ordering.lookup("*");

		var list = new SList<unowned EnchantProvider>();

		if (ordering != null) {
			var tokens = ordering.split(",", 0);
			foreach (unowned string token in tokens) {
				string tok = token.strip();

				foreach (unowned EnchantProvider provider in this.provider_list)
					if (provider != null && tok == provider.identify(provider))
						list.append(provider);
			}
		}

		/* append providers not in the list, or from an unordered list */
		foreach (unowned EnchantProvider provider in this.provider_list)
			if (list.find(provider) == null)
				list.append(provider);

		return list;
	}

	public unowned EnchantDict? request_pwl_dict(string pwl)
			requires (pwl.length > 0)
		{
		this.clear_error();

		/* since the broker pwl file is a read/write file (there is no readonly dictionary associated)
		 * there is no need for complementary exclude file to add a word to. The word just needs to be
		 * removed from the broker pwl file
		 */
		EnchantSession? session = EnchantSession.with_pwl(null, pwl, null, "Personal Wordlist", true);
		if (session == null) {
			this.error = "Couldn't open personal wordlist '%s'".printf(pwl);
			return null;
		}

		session.is_pwl = true;

		unowned var dict = this.new_dict();
		dict.session = session;
		return dict;
	}

	unowned EnchantDict? _request_dict(string tag, string? pwl) {
		SList<unowned EnchantProvider> list = this.get_ordered_providers(tag);
		unowned EnchantDict? dict = null;
		foreach (unowned EnchantProvider provider in list) {
			dict = provider.request_dict(provider, tag);
			if (dict != null) {
				var session = EnchantSession.with_implicit_pwl(provider, tag, pwl);
				dict.session = session;
				break;
			}
		}

		return dict;
	}

	public unowned EnchantDict? request_dict_with_pwl(string composite_tag, string? pwl)
			requires (composite_tag.length > 0)
		{
		// Parse the composite tag and check each is non-empty
		var tags = composite_tag.split(",");
		foreach (unowned string tag in tags)
			if (tag.length == 0)
				return null;

		this.clear_error();

		// Get the dictionaries, and return null if none found
		var dict_list = new SList<weak EnchantDict>();
		foreach (unowned string tag in tags) {
			string normalized_tag = normalize_dictionary_tag(tag);
			unowned EnchantDict dict = this._request_dict(normalized_tag, pwl);
			if (dict == null)
				dict = this._request_dict(iso_639_from_tag(normalized_tag), pwl);
			if (dict == null)
				return null;
			dict_list.append(dict);
		}

		// If there was only one tag, return a single dictionary.
		if (dict_list.length() == 1)
			return dict_list.data;

		// Create the composite dictionary
		var comp_dict = new EnchantCompositeDict();
		comp_dict.dict_list = (owned)dict_list;

		unowned var dict = this.new_dict();
		dict.user_data = (void *)(owned)comp_dict;
		dict.check_method = composite_dict_check;
		dict.suggest_method = composite_dict_suggest;
		dict.add_to_session_method = composite_dict_add_to_session;
		dict.remove_from_session_method = composite_dict_remove_from_session;
		dict.session = EnchantSession.with_implicit_pwl(null, tags[0], pwl);
		return dict;
	}

	public unowned EnchantDict? request_dict(string tag) {
		return this.request_dict_with_pwl(tag, null);
	}

	public void describe(EnchantBrokerDescribeFn fn, void * user_data)
			requires (fn != null)
		{

		this.clear_error();

		foreach (unowned EnchantProvider provider in this.provider_list) {
			string name = provider.identify(provider);
			string desc = provider.describe(provider);
			string file = provider.module.name();

			fn(name, desc, file, user_data);
		}
	}

	public void list_dicts(EnchantDictDescribeFn fn, void *user_data)
			requires (fn != null)
		{
		var tag_map = new HashTable<string, unowned EnchantProvider>(str_hash, str_equal);

		this.clear_error();
		debug("listing dictionaries");
		if (this.provider_list == null)
			debug("no providers found!");

		foreach (unowned EnchantProvider provider in this.provider_list) {
			debug("provider %s", provider.describe(provider));

			foreach (string tag in provider.list_dicts(provider)) {
				debug("tag %s", tag);
				var providers = this.get_ordered_providers(tag);
				int this_priority = providers.index(provider);
				debug("priority %d", this_priority);
				if (this_priority != -1) {
					int min_priority = this_priority + 1;
					unowned var prov = tag_map.lookup(tag);
					if (prov != null)
						min_priority = providers.index(prov);
					if (this_priority < min_priority)
						tag_map.insert(tag, provider);
				}
			}
		}

		var tags = new SList<string>();
		foreach (string key in tag_map.get_keys())
			tags.insert_sorted(key, GLib.strcmp);

		foreach (string tag in tags) {
			unowned var provider = tag_map.lookup(tag);
			string name = provider.identify(provider);
			string desc = provider.describe(provider);
			string file = provider.module.name();
			fn(tag, name, desc, file, user_data);
		}
	}

	public int _dict_exists(string tag)
		/* don't query the providers if it is an empty string */
		requires (tag.length > 0)
		{
		foreach (unowned EnchantProvider provider in this.provider_list)
			if (provider._dictionary_exists(tag) != 0)
				return 1;

		return 0;
	}

	public int dict_exists(string tag)
			requires (tag.length > 0)
		{
		this.clear_error();

		string normalized_tag = normalize_dictionary_tag(tag);
		int exists = 0;

		if ((exists = this._dict_exists(normalized_tag)) == 0) {
			string iso_639_only_tag = iso_639_from_tag(normalized_tag);
			if (iso_639_only_tag == null)
				return 0;

			if (normalized_tag != iso_639_only_tag)
				exists = this._dict_exists(iso_639_only_tag);
		}

		return exists;
	}

	public unowned EnchantDict new_dict() {
		var dict = new EnchantDict();
		this.dicts.add(dict);
		unowned var dict_ref = dict;
		return dict_ref;
	}

	public void free_dict(EnchantDict dict) {
		this.dicts.remove(dict);
		this.clear_error();
	}
}
