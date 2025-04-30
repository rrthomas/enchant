#! /usr/bin/env -S vala --vapidir=src --pkg gio-2.0 slurp.vala
// Test utilities.
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

errordomain TestError {
	TESTERROR;
}

Subprocess start_prog(string prog, string[] args) throws TestError {
	var cmd = new Array<string>.take_zero_terminated(args);
	cmd.prepend_val(prog);
	Subprocess proc = null;
	try {
		proc = new Subprocess.newv(cmd.data,
								   SubprocessFlags.SEARCH_PATH_FROM_ENVP
								   | SubprocessFlags.STDIN_PIPE
								   | SubprocessFlags.STDOUT_PIPE
								   | SubprocessFlags.STDERR_PIPE);
	} catch (Error e) {
		print(@"error starting command $(string.joinv(" ", cmd.data)): $(e.message)\n");
		throw new TestError.TESTERROR("could not run command");
	}
	return proc;
}

struct Output {
	public string std_out;
	public string std_err;
}

Subprocess check_prog(string prog, string[] args) throws Error {
	var proc = start_prog(prog, args);
	proc.wait_check(null);
	return proc;
}

Output run_prog(string prog, string[] args, int expected_rc = 0) {
	string std_out = "";
	string std_err = "";
	try {
		var proc = start_prog(prog, args);
		try {
			proc.wait();
		} catch {}
		if (proc.get_if_exited()) {
			assert_true(proc.get_exit_status() == expected_rc);
		}
		var stdout_pipe = proc.get_stdout_pipe();
		var stderr_pipe = proc.get_stderr_pipe();
		std_out = slurp(stdout_pipe);
		std_err = slurp(stderr_pipe);
	} catch (Error e) {
		print(@"error in command $prog $(string.joinv(" ", args)): $(e.message)\n");
	}
	return Output() { std_out = std_out, std_err = std_err };
}
