/* enchant: miscellaneous public APIs
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


public void enchant_set_prefix_dir(string new_prefix) {
	set_relocation_prefix(INSTALLPREFIX, new_prefix);
}

public unowned string enchant_get_version() {
	return ENCHANT_VERSION_STRING;
}

public string enchant_get_user_language() {
#if OS_WIN32
	return Win32.getlocale();
#else
	string locale = Environment.get_variable("LANG");

	if (locale == null)
		locale = Intl.setlocale(LocaleCategory.MESSAGES);

	if (locale == null)
		locale = Intl.setlocale(LocaleCategory.ALL);

	if (locale == null || locale == "C")
		locale = "en";

	return locale;
#endif /* !OS_WIN32 */
}
