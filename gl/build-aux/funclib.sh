# Set a version string for this script.
scriptversion=2012-10-21.11; # UTC

# General shell script boiler plate, and helper functions.
# Written by Gary V. Vaughan, 2004

# Copyright (C) 2004-2012 Free Software Foundation, Inc.
# This is free software; see the source for copying conditions.  There is NO
# warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# As a special exception to the GNU General Public License, if you distribute
# this file as part of a program or library that is built using GNU Libtool,
# you may include this file under the same distribution terms that you use
# for the rest of that program.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNES FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

# Please report bugs or propose patches to gary@gnu.org.


## ------ ##
## Usage. ##
## ------ ##

# Evaluate this file near the top of your script to gain access to
# the functions and variables defined here:
#
#   . `echo "$0" | ${SED-sed} 's|[^/]*$||'`/build-aux/funclib.sh
#
# If you need to override any of the default environment variable
# settings, do that before evaluating this file.


## -------------------- ##
## Shell normalisation. ##
## -------------------- ##

# Some shells need a little help to be as Bourne compatible as possible.
# Before doing anything else, make sure all that help has been provided!

DUALCASE=1; export DUALCASE # for MKS sh
if test -n "${ZSH_VERSION+set}" && (emulate sh) >/dev/null 2>&1; then :
  emulate sh
  NULLCMD=:
  # Pre-4.2 versions of Zsh do word splitting on ${1+"$@"}, which
  # is contrary to our usage.  Disable this feature.
  alias -g '${1+"$@"}'='"$@"'
  setopt NO_GLOB_SUBST
else
  case `(set -o) 2>/dev/null` in *posix*) set -o posix ;; esac
fi

# NLS nuisances: We save the old values in case they are required later.
_G_user_locale=
_G_safe_locale=
for _G_var in LANG LANGUAGE LC_ALL LC_CTYPE LC_COLLATE LC_MESSAGES
do
  eval "if test set = \"\${$_G_var+set}\"; then
          save_$_G_var=\$$_G_var
          $_G_var=C
	  export $_G_var
	  _G_user_locale=\"$_G_var=\\\$save_\$_G_var; \$_G_user_locale\"
	  _G_safe_locale=\"$_G_var=C; \$_G_safe_locale\"
	fi"
done

# CDPATH.
(unset CDPATH) >/dev/null 2>&1 && unset CDPATH

# Make sure IFS has a sensible default
sp=' '
nl='
'
IFS="	$sp$nl"

# There are still modern systems that have problems with 'echo' mis-
# handling backslashes, among others, so make sure $bs_echo is set to a
# command that correctly interprets backslashes.
# (this code from Autoconf 2.68)

# Printing a long string crashes Solaris 7 /usr/bin/printf.
bs_echo='\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\'
bs_echo=$bs_echo$bs_echo$bs_echo$bs_echo$bs_echo
bs_echo=$bs_echo$bs_echo$bs_echo$bs_echo$bs_echo$bs_echo
# Prefer a ksh shell builtin over an external printf program on Solaris,
# but without wasting forks for bash or zsh.
if test -z "$BASH_VERSION$ZSH_VERSION" \
    && (test "X`print -r -- $bs_echo`" = "X$bs_echo") 2>/dev/null; then
  bs_echo='print -r --'
  bs_echo_n='print -rn --'
elif (test "X`printf %s $bs_echo`" = "X$bs_echo") 2>/dev/null; then
  bs_echo='printf %s\n'
  bs_echo_n='printf %s'
else
  if test "X`(/usr/ucb/echo -n -n $bs_echo) 2>/dev/null`" = "X-n $bs_echo"; then
    bs_echo_body='eval /usr/ucb/echo -n "$1$nl"'
    bs_echo_n='/usr/ucb/echo -n'
  else
    bs_echo_body='eval expr "X$1" : "X\\(.*\\)"'
    bs_echo_n_body='eval
      arg=$1;
      case $arg in #(
      *"$nl"*)
	expr "X$arg" : "X\\(.*\\)$nl";
	arg=`expr "X$arg" : ".*$nl\\(.*\\)"`;;
      esac;
      expr "X$arg" : "X\\(.*\\)" | tr -d "$nl"
    '
    export bs_echo_n_body
    bs_echo_n='sh -c $bs_echo_n_body bs_echo'
  fi
  export bs_echo_body
  bs_echo='sh -c $bs_echo_body bs_echo'
fi


## ------------------------------- ##
## User overridable command paths. ##
## ------------------------------- ##

# All uppercase variable names are used for environment variables.  These
# variables can be overridden by the user before calling a script that
# uses them if a suitable command of that name is not already available
# in the command search PATH.

: ${CP="cp -f"}
: ${ECHO="$bs_echo"}
: ${EGREP="grep -E"}
: ${FGREP="grep -F"}
: ${GREP="grep"}
: ${LN_S="ln -s"}
: ${MAKE="make"}
: ${MKDIR="mkdir"}
: ${MV="mv -f"}
: ${RM="rm -f"}
: ${SED="sed"}
: ${SHELL="${CONFIG_SHELL-/bin/sh}"}


## -------------------- ##
## Useful sed snippets. ##
## -------------------- ##

sed_dirname='s|/[^/]*$||'
sed_basename='s|^.*/||'

# Sed substitution that helps us do robust quoting.  It backslashifies
# metacharacters that are still active within double-quoted strings.
sed_quote_subst='s|\([`"$\\]\)|\\\1|g'

# Same as above, but do not quote variable references.
sed_double_quote_subst='s/\(["`\\]\)/\\\1/g'

# Sed substitution that turns a string into a regex matching for the
# string literally.
sed_make_literal_regex='s|[].[^$\\*\/]|\\&|g'

# Sed substitution that converts a w32 file name or path
# which contains forward slashes, into one that contains
# (escaped) backslashes.  A very naive implementation.
sed_naive_backslashify='s|\\\\*|\\|g;s|/|\\|g;s|\\|\\\\|g'

# Re-'\' parameter expansions in output of sed_double_quote_subst that
# were '\'-ed in input to the same.  If an odd number of '\' preceded a
# '$' in input to sed_double_quote_subst, that '$' was protected from
# expansion.  Since each input '\' is now two '\'s, look for any number
# of runs of four '\'s followed by two '\'s and then a '$'.  '\' that '$'.
_G_bs='\\'
_G_bs2='\\\\'
_G_bs4='\\\\\\\\'
_G_dollar='\$'
sed_double_backslash="\
  s/$_G_bs4/&\\
/g
  s/^$_G_bs2$_G_dollar/$_G_bs&/
  s/\\([^$_G_bs]\\)$_G_bs2$_G_dollar/\\1$_G_bs2$_G_bs$_G_dollar/g
  s/\n//g"


## ----------------- ##
## Global variables. ##
## ----------------- ##

# Except for the global variables explicitly listed below, the following
# functions in the '^func_' namespace, and the '^require_' namespace
# variables initialised in the 'Resource management' section, sourcing
# this file will not pollute your global namespace with anything
# else. There's no portable way to scope variables in Bourne shell
# though, so actually running these functions will sometimes place
# results into a variable named after the function, and often use
# temporary variables in the '^_G_' namespace. If you are careful to
# avoid using those namespaces casually in your sourcing script, things
# should continue to work as you expect. And, of course, you can freely
# overwrite any of the functions or variables defined here before
# calling anything to customize them.

EXIT_SUCCESS=0
EXIT_FAILURE=1
EXIT_MISMATCH=63  # $? = 63 is used to indicate version mismatch to missing.
EXIT_SKIP=77	  # $? = 77 is used to indicate a skipped test to automake.

# Allow overriding, eg assuming that you follow the convention of
# putting '$debug_cmd' at the start of all your functions, you can get
# bash to show function call trace with:
#
#    debug_cmd='eval echo "${FUNCNAME[0]} $*" >&2' bash your-script-name
debug_cmd=${debug_cmd-":"}
exit_cmd=:

# By convention, finish your script with:
#
#    exit $exit_status
#
# so that you can set exit_status to non-zero if you want to indicate
# something went wrong during execution without actually bailing out at
# the point of failure.
exit_status=$EXIT_SUCCESS

# Work around backward compatibility issue on IRIX 6.5. On IRIX 6.4+, sh
# is ksh but when the shell is invoked as "sh" and the current value of
# the _XPG environment variable is not equal to 1 (one), the special
# positional parameter $0, within a function call, is the name of the
# function.
progpath=$0

# The name of this program.
progname=`$bs_echo "$progpath" |$SED "$sed_basename"`

# Make sure we have an absolute progpath for reexecution:
case $progpath in
  [\\/]*|[A-Za-z]:\\*) ;;
  *[\\/]*)
     progdir=`$bs_echo "$progpath" |$SED "$sed_dirname"`
     progdir=`cd "$progdir" && pwd`
     progpath=$progdir/$progname
     ;;
  *)
     _G_IFS=$IFS
     IFS=${PATH_SEPARATOR-:}
     for progdir in $PATH; do
       IFS=$_G_IFS
       test -x "$progdir/$progname" && break
     done
     IFS=$_G_IFS
     test -n "$progdir" || progdir=`pwd`
     progpath=$progdir/$progname
     ;;
esac


## ----------------- ##
## Standard options. ##
## ----------------- ##

# The following options affect the operation of the functions defined
# below, and should be set appropriately depending on run-time para-
# meters passed on the command line.

opt_dry_run=false
opt_quiet=false
opt_verbose=false

# Categories 'all' and 'none' are always available.  Append any others
# you will pass as the first argument to func_warning from your own
# code.
warning_categories=

# By default, display warnings according to 'opt_warning_types'.  Set
# 'warning_func'  to ':' to elide all warnings, or func_fatal_error to
# treat the next displayed warning as a fatal error.
warning_func=func_warn_and_continue

# Set to 'all' to display all warnings, 'none' to suppress all
# warnings, or a space delimited list of some subset of
# 'warning_categories' to display only the listed warnings.
opt_warning_types=all


## -------------------- ##
## Resource management. ##
## -------------------- ##

# This section contains definitions for functions that each ensure a
# particular resource (a file, or a non-empty configuration variable for
# example) is available, and if appropriate to extract default values
# from pertinent package files. Call them using their associated
# 'require_*' variable to ensure that they are executed, at most, once.
#
# It's entirely deliberate that calling these functions can set
# variables that don't obey the namespace limitations obeyed by the rest
# of this file, in order that that they be as useful as possible to
# callers.


# require_term_colors
# -------------------
# Allow display of bold text on terminals that support it.
require_term_colors=func_require_term_colors
func_require_term_colors ()
{
    $debug_cmd

    test -t 1 && {
      # COLORTERM and USE_ANSI_COLORS environment variables take
      # precedence, because most terminfo databases neglect to describe
      # whether color sequences are supported.
      test -n "${COLORTERM+set}" && : ${USE_ANSI_COLORS="1"}

      if test 1 = "$USE_ANSI_COLORS"; then
        # Standard ANSI escape sequences
        tc_reset='[0m'
        tc_bold='[1m';   tc_standout='[7m'
        tc_red='[31m';   tc_green='[32m'
        tc_blue='[34m';  tc_cyan='[36m'
      else
        # Otherwise trust the terminfo database after all.
        test -n "`tput sgr0 2>/dev/null`" && {
          tc_reset=`tput sgr0`
          test -n "`tput bold 2>/dev/null`" && tc_bold=`tput bold`
          tc_standout=$tc_bold
          test -n "`tput smso 2>/dev/null`" && tc_standout=`tput smso`
          test -n "`tput setaf 1 2>/dev/null`" && tc_red=`tput setaf 1`
          test -n "`tput setaf 2 2>/dev/null`" && tc_green=`tput setaf 2`
          test -n "`tput setaf 4 2>/dev/null`" && tc_blue=`tput setaf 4`
          test -n "`tput setaf 5 2>/dev/null`" && tc_cyan=`tput setaf 5`
        }
      fi
    }

    require_term_colors=:
}


## ----------------- ##
## Function library. ##
## ----------------- ##

# This section contains a variety of useful functions to call in your
# scripts. Take note of the portable wrappers for features provided by
# some modern shells, which will fall back to slower equivalents on
# less featureful shells.


# func_append VAR VALUE
# ---------------------
# Append VALUE onto the existing contents of VAR.

  # We should try to minimise forks, especially on Windows where they are
  # unreasonably slow, so skip the feature probes when bash or zsh are
  # being used:
  if test set = "${BASH_VERSION+set}${ZSH_VERSION+set}"; then
    : ${_G_HAVE_ARITH_OP="yes"}
    : ${_G_HAVE_XSI_OPS="yes"}
    # The += operator was introduced in bash 3.1
    case $BASH_VERSION in
      [12].* | 3.0 | 3.0*) ;;
      *)
        : ${_G_HAVE_PLUSEQ_OP="yes"}
        ;;
    esac
  fi

  # _G_HAVE_PLUSEQ_OP
  # Can be empty, in which case the shell is probed, "yes" if += is
  # useable or anything else if it does not work.
  test -z "$_G_HAVE_PLUSEQ_OP" \
    && (eval 'x=a; x+=" b"; test "a b" = "$x"') 2>/dev/null \
    && _G_HAVE_PLUSEQ_OP=yes

if test yes = "$_G_HAVE_PLUSEQ_OP"
then
  # This is an XSI compatible shell, allowing a faster implementation...
  eval 'func_append ()
  {
    $debug_cmd

    eval "$1+=\$2"
  }'
else
  # ...otherwise fall back to using expr, which is often a shell builtin.
  func_append ()
  {
    $debug_cmd

    eval "$1=\$$1\$2"
  }
fi


# func_append_quoted VAR VALUE
# ----------------------------
# Quote VALUE and append to the end of shell variable VAR, separated
# by a space.
if test yes = "$_G_HAVE_PLUSEQ_OP"; then
  eval 'func_append_quoted ()
  {
    $debug_cmd

    func_quote_for_eval "$2"
    eval "$1+=\\ \$func_quote_for_eval_result"
  }'
else
  func_append_quoted ()
  {
    $debug_cmd

    func_quote_for_eval "$2"
    eval "$1=\$$1\\ \$func_quote_for_eval_result"
  }
fi


# func_append_uniq VAR VALUE
# --------------------------
# Append unique VALUE onto the existing contents of VAR, assuming
# entries are delimited by the first character of VALUE.  For example:
#
#   func_append_uniq options " --another-option option-argument"
#
# will only append to $options if " --another-option option-argument "
# is not already present somewhere in $options already (note spaces at
# each end implied by leading space in second argument).
func_append_uniq ()
{
    $debug_cmd

    eval _G_current_value='`$bs_echo $'$1'`'
    _G_delim=`expr "$2" : '\(.\)'`

    case $_G_delim$_G_current_value$_G_delim in
      *"$2$_G_delim"*) ;;
      *) func_append "$@" ;;
    esac
}


# func_arith TERM...
# ------------------
# Set func_arith_result to the result of evaluating TERMs.
  test -z "$_G_HAVE_ARITH_OP" \
    && (eval 'test 2 = $(( 1 + 1 ))') 2>/dev/null \
    && _G_HAVE_ARITH_OP=yes

if test yes = "$_G_HAVE_ARITH_OP"; then
  eval 'func_arith ()
  {
    $debug_cmd

    func_arith_result=$(( $* ))
  }'
else
  func_arith ()
  {
    $debug_cmd

    func_arith_result=`expr "$@"`
  }
fi


# func_basename FILE
# ------------------
# Set func_basename_result to FILE with everything up to and including
# the last / stripped.
if test yes = "$_G_HAVE_XSI_OPS"; then
  # If this shell supports suffix pattern removal, then use it to avoid
  # forking. Hide the definitions single quotes in case the shell chokes
  # on unsupported syntax...
  _b='func_basename_result=${1##*/}'
  _d='case $1 in
        */*) func_dirname_result=${1%/*}$2 ;;
        *  ) func_dirname_result=$3        ;;
      esac'

else
  # ...otherwise fall back to using sed.
  _b='func_basename_result=`$ECHO "$1" |$SED "$sed_basename"`'
  _d='func_dirname_result=`$ECHO "$1"  |$SED "$sed_dirname"`
      if test "X$func_dirname_result" = "X$1"; then
        func_dirname_result=$3
      else
        func_append func_dirname_result "$2"
      fi'
fi

eval 'func_basename ()
{
    $debug_cmd

    '"$_b"'
}'


# func_dirname FILE APPEND NONDIR_REPLACEMENT
# -------------------------------------------
# Compute the dirname of FILE.  If nonempty, add APPEND to the result,
# otherwise set result to NONDIR_REPLACEMENT.
eval 'func_dirname ()
{
    $debug_cmd

    '"$_d"'
}'


# func_dirname_and_basename FILE APPEND NONDIR_REPLACEMENT
# --------------------------------------------------------
# Perform func_basename and func_dirname in a single function
# call:
#   dirname:  Compute the dirname of FILE.  If nonempty,
#             add APPEND to the result, otherwise set result
#             to NONDIR_REPLACEMENT.
#             value returned in "$func_dirname_result"
#   basename: Compute filename of FILE.
#             value retuned in "$func_basename_result"
# For efficiency, we do not delegate to the functions above but instead
# duplicate the functionality here.
eval 'func_dirname_and_basename ()
{
    $debug_cmd

    '"$_b"'
    '"$_d"'
}'


# func_echo ARG...
# ----------------
# Echo program name prefixed message.
func_echo ()
{
    $debug_cmd

    _G_message=$*

    func_echo_IFS=$IFS
    IFS=$nl
    for _G_line in $_G_message; do
      IFS=$func_echo_IFS
      $bs_echo "$progname: $_G_line"
    done
    IFS=$func_echo_IFS
}


# func_echo_all ARG...
# --------------------
# Invoke $ECHO with all args, space-separated.
func_echo_all ()
{
    $ECHO "$*"
}


# func_echo_infix_1 INFIX ARG...
# ------------------------------
# Echo program name, followed by INFIX on the first line, with any
# additional lines not showing INFIX.
func_echo_infix_1 ()
{
    $debug_cmd

    $require_term_colors

    _G_infix=$1; shift
    _G_indent=$_G_infix
    _G_prefix="$progname: $_G_infix: "
    _G_message=$*

    # Strip color escape sequences before counting printable length
    for _G_tc in "$tc_reset" "$tc_bold" "$tc_standout" "$tc_red" "$tc_green" "$tc_blue" "$tc_cyan"
    do
      test -n "$_G_tc" && {
        _G_esc_tc=`$bs_echo "$_G_tc" | sed "$sed_make_literal_regex"`
        _G_indent=`$bs_echo "$_G_indent" | sed "s|$_G_esc_tc||g"`
      }
    done
    _G_indent="$progname: "`echo "$_G_indent" | sed 's|.| |g'`"  " ## exclude from sc_prohibit_nested_quotes

    func_echo_infix_1_IFS=$IFS
    IFS=$nl
    for _G_line in $_G_message; do
      IFS=$func_echo_infix_1_IFS
      $bs_echo "$_G_prefix$tc_bold$_G_line$tc_reset" >&2
      _G_prefix=$_G_indent
    done
    IFS=$func_echo_infix_1_IFS
}


# func_error ARG...
# -----------------
# Echo program name prefixed message to standard error.
func_error ()
{
    $debug_cmd

    $require_term_colors

    func_echo_infix_1 "  $tc_standout${tc_red}error$tc_reset" "$*" >&2
}


# func_fatal_error ARG...
# -----------------------
# Echo program name prefixed message to standard error, and exit.
func_fatal_error ()
{
    $debug_cmd

    func_error "$*"
    exit $EXIT_FAILURE
}


# func_grep EXPRESSION FILENAME
# -----------------------------
# Check whether EXPRESSION matches any line of FILENAME, without output.
func_grep ()
{
    $debug_cmd

    $GREP "$1" "$2" >/dev/null 2>&1
}


# func_len STRING
# ---------------
# Set func_len_result to the length of STRING. STRING may not
# start with a hyphen.
  test -z "$_G_HAVE_XSI_OPS" \
    && (eval 'x=a/b/c;
      test 5aa/bb/cc = "${#x}${x%%/*}${x%/*}${x#*/}${x##*/}"') 2>/dev/null \
    && _G_HAVE_XSI_OPS=yes

if test yes = "$_G_HAVE_XSI_OPS"; then
  eval 'func_len ()
  {
    $debug_cmd

    func_len_result=${#1}
  }'
else
  func_len ()
  {
    $debug_cmd

    func_len_result=`expr "$1" : ".*" 2>/dev/null || echo $max_cmd_len`
  }
fi


# func_mkdir_p DIRECTORY-PATH
# ---------------------------
# Make sure the entire path to DIRECTORY-PATH is available.
func_mkdir_p ()
{
    $debug_cmd

    _G_directory_path=$1
    _G_dir_list=

    if test -n "$_G_directory_path" && test : != "$opt_dry_run"; then

      # Protect directory names starting with '-'
      case $_G_directory_path in
        -*) _G_directory_path=./$_G_directory_path ;;
      esac

      # While some portion of DIR does not yet exist...
      while test ! -d "$_G_directory_path"; do
        # ...make a list in topmost first order.  Use a colon delimited
	# list incase some portion of path contains whitespace.
        _G_dir_list=$_G_directory_path:$_G_dir_list

        # If the last portion added has no slash in it, the list is done
        case $_G_directory_path in */*) ;; *) break ;; esac

        # ...otherwise throw away the child directory and loop
        _G_directory_path=`$ECHO "$_G_directory_path" | $SED -e "$sed_dirname"`
      done
      _G_dir_list=`$ECHO "$_G_dir_list" | $SED 's|:*$||'`

      func_mkdir_p_IFS=$IFS; IFS=:
      for _G_dir in $_G_dir_list; do
	IFS=$func_mkdir_p_IFS
        # mkdir can fail with a 'File exist' error if two processes
        # try to create one of the directories concurrently.  Don't
        # stop in that case!
        $MKDIR "$_G_dir" 2>/dev/null || :
      done
      IFS=$func_mkdir_p_IFS

      # Bail out if we (or some other process) failed to create a directory.
      test -d "$_G_directory_path" || \
        func_fatal_error "Failed to create '$1'"
    fi
}


# func_mktempdir [BASENAME]
# -------------------------
# Make a temporary directory that won't clash with other running
# libtool processes, and avoids race conditions if possible.  If
# given, BASENAME is the basename for that directory.
func_mktempdir ()
{
    $debug_cmd

    _G_template=${TMPDIR-/tmp}/${1-$progname}

    if test : = "$opt_dry_run"; then
      # Return a directory name, but don't create it in dry-run mode
      _G_tmpdir=$_G_template-$$
    else

      # If mktemp works, use that first and foremost
      _G_tmpdir=`mktemp -d "$_G_template-XXXXXXXX" 2>/dev/null`

      if test ! -d "$_G_tmpdir"; then
        # Failing that, at least try and use $RANDOM to avoid a race
        _G_tmpdir=$_G_template-${RANDOM-0}$$

        func_mktempdir_umask=`umask`
        umask 0077
        $MKDIR "$_G_tmpdir"
        umask $func_mktempdir_umask
      fi

      # If we're not in dry-run mode, bomb out on failure
      test -d "$_G_tmpdir" || \
        func_fatal_error "cannot create temporary directory '$_G_tmpdir'"
    fi

    $ECHO "$_G_tmpdir"
}


# func_normal_abspath PATH
# ------------------------
# Remove doubled-up and trailing slashes, "." path components,
# and cancel out any ".." path components in PATH after making
# it an absolute path.
func_normal_abspath ()
{
    $debug_cmd

    # These SED scripts presuppose an absolute path with a trailing slash.
    _G_pathcar='s|^/\([^/]*\).*$|\1|'
    _G_pathcdr='s|^/[^/]*||'
    _G_removedotparts=':dotsl
		s|/\./|/|g
		t dotsl
		s|/\.$|/|'
    _G_collapseslashes='s|/\{1,\}|/|g'
    _G_finalslash='s|/*$|/|'

    # Start from root dir and reassemble the path.
    func_normal_abspath_result=
    func_normal_abspath_tpath=$1
    func_normal_abspath_altnamespace=
    case $func_normal_abspath_tpath in
      "")
        # Empty path, that just means $cwd.
        func_stripname '' '/' "`pwd`"
        func_normal_abspath_result=$func_stripname_result
        return
        ;;
      # The next three entries are used to spot a run of precisely
      # two leading slashes without using negated character classes;
      # we take advantage of case's first-match behaviour.
      ///*)
        # Unusual form of absolute path, do nothing.
        ;;
      //*)
        # Not necessarily an ordinary path; POSIX reserves leading '//'
        # and for example Cygwin uses it to access remote file shares
        # over CIFS/SMB, so we conserve a leading double slash if found.
        func_normal_abspath_altnamespace=/
        ;;
      /*)
        # Absolute path, do nothing.
        ;;
      *)
        # Relative path, prepend $cwd.
        func_normal_abspath_tpath=`pwd`/$func_normal_abspath_tpath
        ;;
    esac

    # Cancel out all the simple stuff to save iterations.  We also want
    # the path to end with a slash for ease of parsing, so make sure
    # there is one (and only one) here.
    func_normal_abspath_tpath=`$ECHO "$func_normal_abspath_tpath" | $SED \
          -e "$_G_removedotparts" -e "$_G_collapseslashes" -e "$_G_finalslash"`
    while :; do
      # Processed it all yet?
      if test / = "$func_normal_abspath_tpath"; then
        # If we ascended to the root using ".." the result may be empty now.
        if test -z "$func_normal_abspath_result"; then
          func_normal_abspath_result=/
        fi
        break
      fi
      func_normal_abspath_tcomponent=`$ECHO "$func_normal_abspath_tpath" | $SED \
          -e "$_G_pathcar"`
      func_normal_abspath_tpath=`$ECHO "$func_normal_abspath_tpath" | $SED \
          -e "$_G_pathcdr"`
      # Figure out what to do with it
      case $func_normal_abspath_tcomponent in
        "")
          # Trailing empty path component, ignore it.
          ;;
        ..)
          # Parent dir; strip last assembled component from result.
          func_dirname "$func_normal_abspath_result"
          func_normal_abspath_result=$func_dirname_result
          ;;
        *)
          # Actual path component, append it.
          func_append func_normal_abspath_result "/$func_normal_abspath_tcomponent"
          ;;
      esac
    done
    # Restore leading double-slash if one was found on entry.
    func_normal_abspath_result=$func_normal_abspath_altnamespace$func_normal_abspath_result
}


# func_notquiet ARG...
# --------------------
# Echo program name prefixed message only when not in quiet mode.
func_notquiet ()
{
    $debug_cmd

    $opt_quiet || func_echo ${1+"$@"}

    # A bug in bash halts the script if the last line of a function
    # fails when set -e is in force, so we need another command to
    # work around that:
    :
}


# func_relative_path SRCDIR DSTDIR
# --------------------------------
# Set func_relative_path_result to the relative path from SRCDIR to DSTDIR.
func_relative_path ()
{
    $debug_cmd

    func_relative_path_result=
    func_normal_abspath "$1"
    func_relative_path_tlibdir=$func_normal_abspath_result
    func_normal_abspath "$2"
    func_relative_path_tbindir=$func_normal_abspath_result

    # Ascend the tree starting from libdir
    while :; do
      # check if we have found a prefix of bindir
      case $func_relative_path_tbindir in
        $func_relative_path_tlibdir)
          # found an exact match
          func_relative_path_tcancelled=
          break
          ;;
        $func_relative_path_tlibdir*)
          # found a matching prefix
          func_stripname "$func_relative_path_tlibdir" '' "$func_relative_path_tbindir"
          func_relative_path_tcancelled=$func_stripname_result
          if test -z "$func_relative_path_result"; then
            func_relative_path_result=.
          fi
          break
          ;;
        *)
          func_dirname $func_relative_path_tlibdir
          func_relative_path_tlibdir=$func_dirname_result
          if test -z "$func_relative_path_tlibdir"; then
            # Have to descend all the way to the root!
            func_relative_path_result=../$func_relative_path_result
            func_relative_path_tcancelled=$func_relative_path_tbindir
            break
          fi
          func_relative_path_result=../$func_relative_path_result
          ;;
      esac
    done

    # Now calculate path; take care to avoid doubling-up slashes.
    func_stripname '' '/' "$func_relative_path_result"
    func_relative_path_result=$func_stripname_result
    func_stripname '/' '/' "$func_relative_path_tcancelled"
    if test -n "$func_stripname_result"; then
      func_append func_relative_path_result "/$func_stripname_result"
    fi

    # Normalisation. If bindir is libdir, return '.' else relative path.
    if test -n "$func_relative_path_result"; then
      func_stripname './' '' "$func_relative_path_result"
      func_relative_path_result=$func_stripname_result
    fi

    test -n "$func_relative_path_result" || func_relative_path_result=.

    :
}


# func_quote_for_eval ARG...
# --------------------------
# Aesthetically quote ARGs to be evaled later.
# This function returns two values:
#   i) func_quote_for_eval_result
#      double-quoted, suitable for a subsequent eval
#  ii) func_quote_for_eval_unquoted_result
#      has just all characters which are still active within double
#      quotes backslashified.
func_quote_for_eval ()
{
    $debug_cmd

    func_quote_for_eval_unquoted_result=
    func_quote_for_eval_result=
    while test 0 -lt $#; do
      case $1 in
        *[\\\`\"\$]*)
	  _G_unquoted_arg=`printf '%s\n' "$1" |$SED "$sed_quote_subst"` ;;
        *)
          _G_unquoted_arg=$1 ;;
      esac
      if test -n "$func_quote_for_eval_unquoted_result"; then
	func_append func_quote_for_eval_unquoted_result " $_G_unquoted_arg"
      else
        func_append func_quote_for_eval_unquoted_result "$_G_unquoted_arg"
      fi

      case $_G_unquoted_arg in
        # Double-quote args containing shell metacharacters to delay
        # word splitting, command substitution and variable expansion
        # for a subsequent eval.
        # Many Bourne shells cannot handle close brackets correctly
        # in scan sets, so we specify it separately.
        *[\[\~\#\^\&\*\(\)\{\}\|\;\<\>\?\'\ \	]*|*]*|"")
          _G_quoted_arg=\"$_G_unquoted_arg\"
          ;;
        *)
          _G_quoted_arg=$_G_unquoted_arg
	  ;;
      esac

      if test -n "$func_quote_for_eval_result"; then
	func_append func_quote_for_eval_result " $_G_quoted_arg"
      else
        func_append func_quote_for_eval_result "$_G_quoted_arg"
      fi
      shift
    done
}


# func_quote_for_expand ARG
# -------------------------
# Aesthetically quote ARG to be evaled later; same as above,
# but do not quote variable references.
func_quote_for_expand ()
{
    $debug_cmd

    case $1 in
      *[\\\`\"]*)
	_G_arg=`$ECHO "$1" | $SED \
	    -e "$sed_double_quote_subst" -e "$sed_double_backslash"` ;;
      *)
        _G_arg=$1 ;;
    esac

    case $_G_arg in
      # Double-quote args containing shell metacharacters to delay
      # word splitting and command substitution for a subsequent eval.
      # Many Bourne shells cannot handle close brackets correctly
      # in scan sets, so we specify it separately.
      *[\[\~\#\^\&\*\(\)\{\}\|\;\<\>\?\'\ \	]*|*]*|"")
        _G_arg=\"$_G_arg\"
        ;;
    esac

    func_quote_for_expand_result=$_G_arg
}


# func_stripname PREFIX SUFFIX NAME
# ---------------------------------
# strip PREFIX and SUFFIX from NAME, and store in func_stripname_result.
# PREFIX and SUFFIX must not contain globbing or regex special
# characters, hashes, percent signs, but SUFFIX may contain a leading
# dot (in which case that matches only a dot).
if test yes = "$_G_HAVE_XSI_OPS"; then
  eval 'func_stripname ()
  {
    $debug_cmd

    # pdksh 5.2.14 does not do ${X%$Y} correctly if both X and Y are
    # positional parameters, so assign one to ordinary variable first.
    func_stripname_result=$3
    func_stripname_result=${func_stripname_result#"$1"}
    func_stripname_result=${func_stripname_result%"$2"}
  }'
else
  func_stripname ()
  {
    $debug_cmd

    case $2 in
      .*) func_stripname_result=`$ECHO "$3" | $SED -e "s%^$1%%" -e "s%\\\\$2\$%%"`;;
      *)  func_stripname_result=`$ECHO "$3" | $SED -e "s%^$1%%" -e "s%$2\$%%"`;;
    esac
  }
fi


# func_show_eval CMD [FAIL_EXP]
# -----------------------------
# Unless opt_quiet is true, then output CMD.  Then, if opt_dryrun is
# not true, evaluate CMD.  If the evaluation of CMD fails, and FAIL_EXP
# is given, then evaluate it.
func_show_eval ()
{
    $debug_cmd

    _G_cmd=$1
    _G_fail_exp=${2-':'}

    func_quote_for_expand "$_G_cmd"
    eval "func_notquiet $func_quote_for_expand_result"

    $opt_dry_run || {
      eval "$_G_cmd"
      _G_status=$?
      if test 0 -ne "$_G_status"; then
	eval "(exit $_G_status); $_G_fail_exp"
      fi
    }
}


# func_show_eval_locale CMD [FAIL_EXP]
# ------------------------------------
# Unless opt_quiet is true, then output CMD.  Then, if opt_dryrun is
# not true, evaluate CMD.  If the evaluation of CMD fails, and FAIL_EXP
# is given, then evaluate it.  Use the saved locale for evaluation.
func_show_eval_locale ()
{
    $debug_cmd

    _G_cmd=$1
    _G_fail_exp=${2-':'}

    $opt_quiet || {
      func_quote_for_expand "$_G_cmd"
      eval "func_echo $func_quote_for_expand_result"
    }

    $opt_dry_run || {
      eval "$_G_user_locale
	    $_G_cmd"
      _G_status=$?
      eval "$_G_safe_locale"
      if test 0 -ne "$_G_status"; then
	eval "(exit $_G_status); $_G_fail_exp"
      fi
    }
}


# func_tr_sh
# ----------
# Turn $1 into a string suitable for a shell variable name.
# Result is stored in $func_tr_sh_result.  All characters
# not in the set a-zA-Z0-9_ are replaced with '_'. Further,
# if $1 begins with a digit, a '_' is prepended as well.
func_tr_sh ()
{
    $debug_cmd

    case $1 in
    [0-9]* | *[!a-zA-Z0-9_]*)
      func_tr_sh_result=`$ECHO "$1" | $SED -e 's/^\([0-9]\)/_\1/' -e 's/[^a-zA-Z0-9_]/_/g'`
      ;;
    * )
      func_tr_sh_result=$1
      ;;
    esac
}


# func_verbose ARG...
# -------------------
# Echo program name prefixed message in verbose mode only.
func_verbose ()
{
    $debug_cmd

    $opt_verbose && func_echo "$*"

    :
}


# func_warn_and_continue ARG...
# -----------------------------
# Echo program name prefixed warning message to standard error.
func_warn_and_continue ()
{
    $debug_cmd

    $require_term_colors

    func_echo_infix_1 "${tc_red}warning$tc_reset" "$*" >&2
}


# func_warning CATEGORY ARG...
# ----------------------------
# Echo program name prefixed warning message to standard error. Warning
# messages can be filtered according to CATEGORY, where this function
# elides messages where CATEGORY is not listed in the global variable
# 'opt_warning_types'.
func_warning ()
{
    $debug_cmd

    # CATEGORY must be in the warning_categories list!
    case " $warning_categories " in
      *" $1 "*) ;;
      *) func_internal_error "invalid warning category '$1'" ;;
    esac

    _G_category=$1
    shift

    case " $opt_warning_types " in
      *" $_G_category "*) $warning_func ${1+"$@"} ;;
    esac
}


# Local variables:
# mode: shell-script
# sh-indentation: 2
# eval: (add-hook 'write-file-hooks 'time-stamp)
# time-stamp-pattern: "10/scriptversion=%:y-%02m-%02d.%02H; # UTC"
# time-stamp-time-zone: "UTC"
# End:
