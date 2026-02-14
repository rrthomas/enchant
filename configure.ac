# configure.ac for Enchant
# Process this file with autoconf to produce a configure script
#
# Copyright (C) 2003 Dom Lachowicz
#               2016-2025 Reuben Thomas
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.

AC_PREREQ([2.71])
AC_INIT([Enchant],[2.8.15])
AC_CONFIG_SRCDIR(lib/enchant.h)
AC_CONFIG_AUX_DIR([build-aux])
AC_REQUIRE_AUX_FILE([tap-driver.sh])
AM_INIT_AUTOMAKE([subdir-objects tar-ustar])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])

dnl Release number rules:
dnl This is derived from "Versioning" chapter of info libtool documentation.
dnl Format is MAJOR:MINOR:MICRO
dnl     4a) Increment major when removing or changing interfaces.
dnl     4a) 5) Increment minor when adding interfaces.
dnl     6) Set minor to zero when removing or changing interfaces.
dnl     3) Increment micro when interfaces not changed at all,
dnl               only bug fixes or internal changes made.
dnl     4b) Set micro to zero when adding, removing or changing interfaces.

dnl First extract pieces from the version number string
ENCHANT_MAJOR_VERSION=`echo $VERSION | awk -F. '{print $1}'`
ENCHANT_MINOR_VERSION=`echo $VERSION | awk -F. '{print $2}'`
ENCHANT_MICRO_VERSION=`echo $VERSION | awk -F. '{print $3}'`
ENCHANT_VERSION=$ENCHANT_MAJOR_VERSION.$ENCHANT_MINOR_VERSION.$ENCHANT_MICRO_VERSION$ENCHANT_MICRO_VERSION_SUFFIX
ENCHANT_VERSION_NUMBER=`expr $ENCHANT_MAJOR_VERSION \* 1000000 + $ENCHANT_MINOR_VERSION \* 1000 + $ENCHANT_MICRO_VERSION`

ENCHANT_SONUM=$ENCHANT_MAJOR_VERSION
AGE=$ENCHANT_MINOR_VERSION
REVISION=$ENCHANT_MICRO_VERSION
CURRENT=`expr $ENCHANT_SONUM + $AGE`

dnl Version info for libraries = CURRENT:REVISION:AGE
VERSION_INFO=$CURRENT:$REVISION:$AGE

AC_SUBST(VERSION_INFO)
AC_SUBST(ENCHANT_MAJOR_VERSION)
AC_SUBST(ENCHANT_MINOR_VERSION)
AC_SUBST(ENCHANT_MICRO_VERSION)

dnl Checks for programs.
AC_PROG_CC
gl_EARLY
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(11)
AC_PROG_OBJCXX
PKG_PROG_PKG_CONFIG
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MKDIR_P
LT_INIT([win32-dll])
gl_INIT
AC_SUBST([objdir])
LT_SYS_MODULE_EXT
AC_SUBST([shlibext], [$libltdl_cv_shlibext])
AC_CONFIG_LIBOBJ_DIR([lib])
AC_PATH_PROG([DOXYGEN],[doxygen],[AC_MSG_ERROR([Doxygen not found])])

dnl Vala
AX_CHECK_GNU_MAKE(,[AC_MSG_WARN([GNU make is required to build from Vala sources])])
AM_PROG_VALAC(0.56,,[AC_MSG_WARN([valac not found or too old: cannot compile Vala sources])])

dnl Glib and friends
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.76 gmodule-2.0 gobject-2.0 gio-2.0])

dnl Extra warnings with GCC and compatible compilers
AC_ARG_ENABLE([gcc-warnings],
  [AS_HELP_STRING([--disable-gcc-warnings],
                  [turn off lots of GCC warnings])],
  [case $enableval in
     yes|no) ;;
     *)      AC_MSG_ERROR([bad value $enableval for gcc-warnings option]) ;;
   esac
   gl_gcc_warnings=$enableval],
  [gl_gcc_warnings=yes]
)
if test "$gl_gcc_warnings" = yes; then
  dnl Set up the list of undesired warnings.
  nw=
  nw="$nw -Wsystem-headers"   # Don’t let system headers trigger warnings
  nw="$nw -Wabi-tag"          # Gives warnings from library headers.
  nw="$nw -Wold-style-cast"   # Warning from glib macros like g_new0
  nw="$nw -Wuseless-cast"     # Likewise

  gl_MANYWARN_ALL_GCC([warnings])

  dnl Enable all GCC warnings not in this list.
  gl_MANYWARN_COMPLEMENT([warnings], [$warnings], [$nw])
  for w in $warnings; do
    gl_WARN_ADD([$w])
  done

  AC_LANG_PUSH([C++])
  gl_MANYWARN_ALL_GCC([cxx_warnings])

  dnl Enable all G++ warnings not in this list.
  gl_MANYWARN_COMPLEMENT([cxx_warnings], [$cxx_warnings], [$nw])
  for w in $cxx_warnings; do
    gl_WARN_ADD([$w])
  done
  AC_LANG_POP

  # Some warnings triggered by Vala-generated code.
  AC_LANG_PUSH([C])
  nw=
  nw="$nw -Wshadow"
  nw="$nw -Wnull-dereference"
  nw="$nw -Wall"
  nw="$nw -Wunused-macros"
  nw="$nw -Wincompatible-pointer-types"
  nw="$nw -fanalyzer"
  nw="$nw -Wsuggest-attribute=const"
  nw="$nw -Wsuggest-attribute=malloc"
  gl_MANYWARN_ALL_GCC([cvala_warnings])
  gl_MANYWARN_COMPLEMENT([cvala_warnings], [$cvala_warnings], [$nw])
  for w in $cvala_warnings; do
    gl_WARN_ADD([$w], [WARN_VALA_CFLAGS])
  done
  gl_WARN_ADD([-Wno-incompatible-pointer-types], [WARN_VALA_CFLAGS])
  AC_LANG_POP

  # When compiling with GCC, prefer -isystem to -I when including system
  # include files, to avoid generating useless diagnostics for the files.
  ISYSTEM='-isystem '
else
  ISYSTEM='-I'
fi
AC_SUBST([ISYSTEM])

AM_CONDITIONAL([SHLIBS_IN_BINDIR], [case "$host_os" in mingw* | cygwin*) true;; *) false;; esac])

dnl Windows-specific checks
AC_MSG_CHECKING([for native Win32])
native_win32=no
case "$host" in
  *-*-mingw*)
    native_win32=yes
    ;;
esac
AC_MSG_RESULT([$native_win32])
AM_CONDITIONAL(OS_WIN32, test "x$native_win32" = "xyes")

dnl ===========================================================================

AC_LANG_PUSH([C++])
PKG_CHECK_EXISTS([UnitTest++],
    [PKG_CHECK_MODULES(UNITTESTPP, [UnitTest++ >= 1.6])])
AC_LANG_POP([C++])


dnl Provider detection
AC_DEFUN([ENCHANT_CHECK_PROVIDER_SETUP],
  [m4_define([provider_check], [m4_default([$2],[check])])
   AC_ARG_WITH([$1],
   AS_HELP_STRING([--with-[]$1],
      [enable the $1 provider @<:@default=provider_check@:>@]),
      [with_[]$1=$withval],
      [with_[]$1=provider_check])
   $1[]_dir=${datadir}/$1])

AC_DEFUN([ENCHANT_CHECK_PKG_CONFIG_PROVIDER],
  [ENCHANT_CHECK_PROVIDER_SETUP([$1], [$4])
   AS_IF([test "$with_[]$1" != no],
         [PKG_CHECK_MODULES([$2], [m4_default([$3], [$1])],
         [$2[]_CFLAGS="$[]$2[]_CFLAGS -DENCHANT_[]$2[]_DICT_DIR='\"$[]$1_dir\"'"
         with_$1=yes
         build_providers="$build_providers $1"],
         [if test "$with_$1" != check; then
            AC_MSG_FAILURE([--with-[]$1 was given, but test(s) for $1 failed])
          fi
          with_$1=no])])
   AM_CONDITIONAL(WITH_[]$2, test "$with_[]$1" = yes)])

AC_DEFUN([ENCHANT_CHECK_LIB_PROVIDER],
  [ENCHANT_CHECK_PROVIDER_SETUP([$1], [$4])
   AS_IF([test "$with_[]$1" != no],
      [if test "$3" != NOLIB; then
         AC_CHECK_LIB([$1], [$3],,, [$5])
      else
         ac_cv_lib_[]$1[]_[]$3=yes
      fi
      if test "$HAVE_[]$6" != yes -o "$ac_cv_lib_[]$1[]_[]$3" != yes; then
         if test "$with_[]$1" != check; then
            AC_MSG_FAILURE([--with-[]$1 was given, but tests for $1 failed])
         fi
         with_[]$1=no
      else
         with_[]$1=yes
         build_providers="$build_providers $1"
         $2[]_LIBS="$5"
         AC_SUBST([$2_LIBS])
      fi])
   AM_CONDITIONAL(WITH_[]$2, test "$with_[]$1" = yes)])


dnl Check for providers
build_providers=

dnl Standard providers
AC_CHECK_HEADERS([aspell.h], [HAVE_ASPELL_H=yes])
AC_CHECK_HEADERS([hspell.h], [HAVE_HSPELL_H=yes])
ENCHANT_CHECK_PKG_CONFIG_PROVIDER([hunspell], [HUNSPELL])
ENCHANT_CHECK_PKG_CONFIG_PROVIDER([nuspell], [NUSPELL], [nuspell >= 5.1.0])
ENCHANT_CHECK_LIB_PROVIDER([aspell], [ASPELL], [get_aspell_dict_info_list],,, [ASPELL_H])
ENCHANT_CHECK_LIB_PROVIDER([hspell], [HSPELL], [hspell_get_dictionary_path],, [-lz], [HSPELL_H])
ENCHANT_CHECK_PKG_CONFIG_PROVIDER([voikko], [VOIKKO], [libvoikko])

dnl Windows 8 Spell Checking API provider
dnl TODO: The Windows version is untested and assumed to be at least Windows 8.
if test "$native_win32" = "yes"; then
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADERS([spellcheck.h], [HAVE_SPELLCHECK_H=yes])
  ENCHANT_CHECK_LIB_PROVIDER([win8], [WIN8], [NOLIB],, [-lole32 -loleaut32 -luuid], [SPELLCHECK_H])
  AC_LANG_POP([C++])
fi
dnl Must call AM_CONDITIONAL outside conditional
AM_CONDITIONAL(WITH_WIN8, test "$with_win8" = yes)
dnl Add Windows target to CFLAGS.
AS_IF([test "$with_win8" = yes],
  [WIN8_CFLAGS="-D_WIN32_WINNT=_WIN32_WINNT_WIN8 -DNTDDI_VERSION=NTDDI_WIN8" AC_SUBST([WIN8_CFLAGS])])

dnl FIXME: The test below assumes GCC(-compatible) ObjC++ compiler, but
dnl OBJCXX is set even if no compiler is found.
if test "$ac_cv_objcxx_compiler_gnu" = "yes"; then
  AC_LANG_PUSH([Objective C++])
  AC_CHECK_HEADERS([Cocoa/Cocoa.h], [HAVE_COCOA_COCOA_H=yes])
  ENCHANT_CHECK_LIB_PROVIDER([applespell], [APPLESPELL], [NOLIB],, [-framework Cocoa], [COCOA_COCOA_H])
  AC_LANG_POP([Objective C++])
fi
dnl Must call AM_CONDITIONAL outside conditional
AM_CONDITIONAL(WITH_APPLESPELL, test "$with_applespell" = yes)

dnl Experimental/deprecated providers
ENCHANT_CHECK_PKG_CONFIG_PROVIDER([zemberek], [ZEMBEREK], [dbus-glib-1 >= 0.62], [no])

dnl Counting code
AM_EXTRA_RECURSIVE_TARGETS([loc])
AC_PATH_PROG(CLOC, cloc, true)
CLOC_OPTS="--autoconf --force-lang=C,h --force-lang='Bourne Shell',conf"
AC_SUBST([CLOC_OPTS])

dnl =======================================================================================

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
Doxyfile
Makefile
enchant-$ENCHANT_MAJOR_VERSION.pc:enchant.pc.in
lib/Makefile
lib/enchant.5.in
libgnu/Makefile
program-tests/Makefile
providers/Makefile
src/Makefile
src/enchant.1
src/enchant-lsmod.1
tests/Makefile
], [],
[ENCHANT_MAJOR_VERSION="$ENCHANT_MAJOR_VERSION"])
AC_OUTPUT

dnl ===========================================================================================

echo "Providers to build:${build_providers}"
if test "$build_providers" = ""; then
   AC_MSG_WARN([No spell-checking provider selected!])
fi

if test "$with_zemberek" = yes; then
   echo "
The Zemberek Turkish spell-checking provider is enabled. It is known
to cause crashes with WebKit. Use at your own discretion."
fi
if test $RELOCATABLE != yes; then
  AC_MSG_WARN([Building without --enable-relocatable: the tests will not work!])
fi
if test "$datadir" != '${datarootdir}'; then
  AC_MSG_WARN([--datadir set: the tests will not work!])
fi
