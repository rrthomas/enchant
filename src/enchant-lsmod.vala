/* enchant
 * Copyright (C) 2003 Dom Lachowicz
 * Copyright (C) 2017-2024 Reuben Thomas <rrt@sc3d.org>
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
 * the non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers. If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

using Enchant;

void describe_dict(string lang_tag,
				   string provider_name,
				   string provider_desc,
				   string provider_file) {
	print("%s (%s)\n", lang_tag, provider_name);
}

void describe_word_chars(string lang_tag,
						 string provider_name,
						 string provider_desc,
						 string provider_file,
						 Dict self) {
	string word_chars = "";
	if (self != null)
		word_chars = self.get_extra_word_characters();
    print("%s\n", word_chars != null ? word_chars : "");
}

void describe_provider(string name, string desc, string file) {
	print("%s (%s)\n", name, desc);
}

void usage(string progname) {
	stderr.printf("%s [[-lang|-word-chars] [language_tag]|-list-dicts|-help|-version]\n", progname);
}

public int main(string[] args) {
	var broker = new Broker();
	string lang_tag = null;
	int retcode = 0;

	if (args.length > 1) {
		if (args[1] == "-lang" || args[1] == "-word-chars") {
			if (args.length > 2) {
				lang_tag = args[2];
			} else {
				lang_tag = enchant_get_user_language();
				if (lang_tag == null || lang_tag == "C")
					lang_tag = "en";
			}
			if (lang_tag == null) {
				stderr.printf("Error: language tag not specified and no default language set\n");
				retcode = 1;
			} else {
				unowned var dict = broker.request_dict(lang_tag);
				if (dict == null) {
					stderr.printf("No dictionary available for '%s'", lang_tag);
					string errmsg = broker.get_error();
					if (errmsg != null)
						stderr.printf(": %s", errmsg);
					stderr.putc('\n');
					retcode = 1;
				} else {
					DictDescribeFn fn;
					if (args[1] == "-lang")
						fn = describe_dict;
					else
						fn = describe_word_chars;
					dict.describe(fn, dict);
				}
			}
		} else if (args[1] == "-h" || args[1] == "-help") {
			usage(args[0]);
		} else if (args[1] == "-v" || args[1] == "-version") {
			stderr.printf("%s %s\n", args[0], PACKAGE_VERSION);
		} else if (args[1] == "-list-dicts") {
			broker.list_dicts(describe_dict);
		} else {
			stderr.printf("Invalid argument %s\n", args[1]);
			usage(args[0]);
			retcode = 1;
		}
	} else
		broker.describe(describe_provider);

	return retcode;
}
