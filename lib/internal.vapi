/* Bits of enchant.h that we need in Vala
 * Copyright (C) 2024 Reuben Thomas <rrt@sc3d.org>
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

[CCode (has_target = false, cheader_filename = "enchant.h")]
public delegate void EnchantBrokerDescribeFn(string provider_name,
											 string provider_desc,
											 string provider_dll_file,
											 void *user_data);

[CCode (has_target = false, cheader_filename = "enchant.h")]
public delegate void EnchantDictDescribeFn(string lang_tag,
										   string provider_name,
										   string provider_desc,
										   string provider_file,
										   void *user_data);


// Vala maps size_t to gsize and ssize_t to gssize by default, but these
// types are not necessarily the same as size_t and ssize_t.  Hence, reuse
// the definitions of size_t and ssize_t from posix.vapi used in the 'posix'
// profile.

[CCode (cname = "size_t", cheader_filename = "sys/types.h", default_value = "0UL")]
[IntegerType (rank = 9)]
public struct real_size_t {
	public inline string to_string () {
		return "%zu".printf (this);
	}
}

[CCode (cname = "ssize_t", cheader_filename = "sys/types.h", default_value = "0L")]
[IntegerType (rank = 8)]
public struct real_ssize_t {
	public inline string to_string () {
		return "%zi".printf (this);
	}
}


// Bind g_strfreev in a way that is compatible with use on char **.
[CCode (cheader_filename = "glib.h")]
public void g_strfreev ([CCode (array_length = false, array_null_terminated = true)] owned string[] str_array);
