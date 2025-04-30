#! /usr/bin/env -S vala --pkg posix
// Slurp a file.
//
// © 2025 Reuben Thomas <rrt@sc3d.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <https://www.gnu.org/licenses/>.

using Posix;

public string slurp (InputStream stream) throws Error {
	uint8[] data = {};
	ulong buf_size = 65536;
	uint8[] buf = new uint8[buf_size];
	ssize_t bytes;
	ulong total = 0;
	while (true) {
		bytes = stream.read (buf);
		if (bytes <= 0)
			break;
		ulong size = (ulong) (total + bytes);
		if (size > data.length) {
			ulong new_len = buf_size * 2;
			uint8[] prev = data;
			data = new uint8[new_len];
			Memory.copy (&data[0], &prev[0], total);
			prev = null;
		}
		Memory.copy (&data[0] + total, &buf[0], bytes);
		total += (ulong) bytes;
	}
	return total == 0 ? "" : (string) data[0 : total];
}
