# configure.ac for Enchant
# Process this file with autoconf to produce a configure script
#
# Copyright (C) 2003 Dom Lachowicz
#               2016-2024 Reuben Thomas
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.

AC_PREREQ([2.71])
AC_INIT([enchant],[2.6.9])
AC_CONFIG_SRCDIR(src/enchant.h)
AC_CONFIG_AUX_DIR([build-aux])
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
LT_LANG([Windows Resource])
gl_INIT
AC_SUBST([objdir])
LT_SYS_MODULE_EXT
AC_SUBST([shlibext], [$libltdl_cv_shlibext])
AC_CONFIG_LIBOBJ_DIR([lib])
AC_PATH_PROG([DOXYGEN],[doxygen],[AC_MSG_ERROR([Doxygen not found])])

dnl Vala
AX_CHECK_GNU_MAKE(,[AC_MSG_WARN([GNU make is required to build from Vala sources])])
AM_PROG_VALAC(0.52,,[AC_MSG_WARN([valac not found or too old: cannot compile Vala sources])])

dnl Glib and friends
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.6 gmodule-2.0 gobject-2.0 gio-2.0])

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
  nw="$nw -Wimplicit-fallthrough=5" # We want to allow "fallthrough" comments.
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

  # When compiling with GCC, prefer -isystem to -I when including system
  # include files, to avoid generating useless diagnostics for the files.
  ISYSTEM='-isystem '
else
  ISYSTEM='-I'
fi
AC_SUBST([ISYSTEM])

AC_MSG_CHECKING([for native Win32])
native_win32=no
case "$host" in
  *-*-mingw*)
    native_win32=yes
    ;;
esac
AC_MSG_RESULT([$native_win32])
AM_CONDITIONAL(OS_WIN32, test "$native_win32" = yes)

AM_CONDITIONAL([SHLIBS_IN_BINDIR], [case "$host_os" in mingw* | cygwin*) true;; *) false;; esac])

dnl ===========================================================================

m4_copy([AC_DEFUN],[glib_DEFUN])
glib_DEFUN([GLIB_LC_MESSAGES],
  [AC_CHECK_HEADERS([locale.h])
    if test $ac_cv_header_locale_h = yes; then
    AC_CACHE_CHECK([for LC_MESSAGES], am_cv_val_LC_MESSAGES,
      [AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <locale.h>]], [[return LC_MESSAGES]])],[am_cv_val_LC_MESSAGES=yes],[am_cv_val_LC_MESSAGES=no])])
    if test $am_cv_val_LC_MESSAGES = yes; then
      AC_DEFINE(HAVE_LC_MESSAGES, 1,
        [Define if your <locale.h> file defines LC_MESSAGES.])
    fi
  fi])

GLIB_LC_MESSAGES

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
         LIBS="$LIBS $5"
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

dnl =======================================================================================

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
Makefile
enchant-$ENCHANT_MAJOR_VERSION.pc:enchant.pc.in
lib/Makefile
src/libenchant.rc
src/Makefile
src/enchant.1
src/enchant-lsmod.1
src/enchant.5.in
providers/Makefile
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
The Zemberek Turkish spell-checking plugin is enabled. It is known
to cause crashes with WebKit. Use at your own discretion."
fi
if test $RELOCATABLE != yes; then
  AC_MSG_WARN([Building without --enable-relocatable: the tests will not work!])
fi
if test "$datadir" != '${datarootdir}'; then
  AC_MSG_WARN([--datadir set: the tests will not work!])
fi
