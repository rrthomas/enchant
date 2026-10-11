/* gnu.vapi
 *
 * Copyright (C) 2020, 2024 Reuben Thomas
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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 * Author:
 *	Reuben Thomas <rrt@sc3d.org>
 */

[CCode (cprefix = "", lower_case_cprefix = "")]
namespace Gnu {
	/**
	 * Provides the values for the 'has_arg' field of 'GetoptOption'.
	 */
	[CCode (cname = "int", cprefix = "", cheader_filename = "getopt.h", has_type_id = false)]
	public enum GetoptArgument {
		[CCode (cname = "no_argument")]
		NONE,
		[CCode (cname = "required_argument")]
		REQUIRED,
		[CCode (cname = "optional_argument")]
		OPTIONAL
	}

	[CCode (cname = "struct option", cheader_filename = "getopt.h", has_type_id = false)]
	public struct GetoptOption {
		public unowned string name;
		public int has_arg;
		public int *flag;
		public int val;
	}

	[CCode (cheader_filename = "getopt.h")]
	public int getopt_long ([CCode (array_length_pos = 0.1)] string[] args, string shortopts, [CCode (array_length = false, array_null_terminated = true)] GetoptOption[] longopts, out int longind);
	[CCode (cheader_filename = "getopt.h")]
	public int getopt_long_only ([CCode (array_length_pos = 0.1)] string[] args, string shortopts, [CCode (array_length = false, array_null_terminated = true)] GetoptOption[] longopts, out int longind);


	[CCode (cheader_filename = "string.h", feature_test_macro = "_GNU_SOURCE")]
	public string memmem (string haystack, size_t haystack_len, string needle, size_t needle_len);

	[CCode (cheader_filename = "error.h")]
	[PrintfFormat]
	public void error (int status, int errnum, string format, ...);

	/**
	 * Provides the values for the 'operation' argument of 'flock'.
	 */
	[CCode (cname = "int", cprefix = "LOCK_", cheader_filename = "sys/file.h", has_type_id = false)]
	public enum FlockOperation {
		SH,
		EX,
		UN
	}
	[CCode (cheader_filename = "sys/file.h")]
	public int flock (int fd, FlockOperation operation);

	[CCode (cheader_filename = "binary-io.h")]
	public int set_binary_mode(int fd, int mode);

	[CCode (cheader_filename = "fcntl.h")]
	public const int O_BINARY;
	[CCode (cheader_filename = "fcntl.h")]
	public const int O_TEXT;

	[CCode (cheader_filename = "stat-time.h")]
	public long get_stat_atime_ns (Posix.Stat st);
	[CCode (cheader_filename = "stat-time.h")]
	public long get_stat_ctime_ns (Posix.Stat st);
	[CCode (cheader_filename = "stat-time.h")]
	public long get_stat_mtime_ns (Posix.Stat st);
	[CCode (cheader_filename = "stat-time.h")]
	public long get_stat_birthtime_ns (Posix.Stat st);

	[SimpleType]
	[CCode (cname = "struct timespec_value", cheader_filename = "time.h", has_type_id = false)]
	public struct timespec_value {
		public time_t tv_sec;
		public long tv_nsec;
	}
	[CCode (cheader_filename = "stat-time.h")]
	public timespec_value get_stat_atime (Posix.Stat st);
	[CCode (cheader_filename = "stat-time.h")]
	public timespec_value get_stat_ctime (Posix.Stat st);
	[CCode (cheader_filename = "stat-time.h")]
	public timespec_value get_stat_mtime (Posix.Stat st);
	[CCode (cheader_filename = "stat-time.h")]
	public timespec_value get_stat_birthtime (Posix.Stat st);

	[CCode (cheader_filename = "quote.h")]
	public string quote (string arg);

	[CCode (cheader_filename = "relocatable.h", cname = "relocate", type = "const char*")]
	char* _gnulib_relocate (char* path);
	[CCode (cname = "_vala_gnulib_relocate")]
	public string relocate (string path) {
		char* newpath = _gnulib_relocate (path);
		if (newpath != path) {
			// If relocate malloced, then return the value, defeating Vala's
			// attempt to strdup it.
			return (string) (owned) newpath;
		} else {
			// Otherwise, allow Vala to strdup the non-malloced return value.
			return (string) newpath;
		}
	}

	[CCode (cheader_filename = "relocatable.h")]
	public void set_relocation_prefix (string orig_prefix, string curr_prefix);

	[CCode (cheader_filename = "relocatable.h")]
	public string compute_curr_prefix (string orig_installprefix, string orig_installdir, string curr_pathname);
}
