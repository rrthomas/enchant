AC_INIT([enchant],[2.3.2])
AC_CONFIG_SRCDIR(src/enchant.h)
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([subdir-objects])
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
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MKDIR_P
AC_HEADER_STDC
LT_INIT([win32-dll])
LT_LANG([Windows Resource])
gl_INIT
gl_GNU_MAKE
AC_SUBST([objdir])
LT_SYS_MODULE_EXT
AC_SUBST([shlibext], [$libltdl_cv_shlibext])
AC_CONFIG_LIBOBJ_DIR([lib])


PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.6 gmodule-2.0])

# FIXME: Use just GLIB_CFLAGS, GLIB_LIBS
ENCHANT_CFLAGS=$GLIB_CFLAGS
ENCHANT_LIBS=$GLIB_LIBS

AC_SUBST(ENCHANT_CFLAGS)
AC_SUBST(ENCHANT_LIBS)

AC_MSG_CHECKING([for native Win32])
native_win32=no
case "$host" in
  *-*-mingw*)
    native_win32=yes
    ;;
esac
AC_MSG_RESULT([$native_win32])
AM_CONDITIONAL(OS_WIN32, test "x$native_win32" = "xyes")

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
    [PKG_CHECK_MODULES(UNITTESTPP, [UnitTest++])])
AC_SUBST(UNITTESTPP_CFLAGS)
AC_SUBST(UNITTESTPP_LIBS)
AC_LANG_POP([C++])


dnl Provider detection
AC_DEFUN([ENCHANT_CHECK_PROVIDER_SETUP],
  [m4_define([provider_check], [m4_default([$2],[check])])
   AC_ARG_WITH([$1],
   AS_HELP_STRING([--with-[]$1],
      [enable the $1 provider @<:@default=provider_check@:>@]),
      [with_[]$1=$withval],
      [with_[]$1=provider_check])
   $1[]_dir=${datadir}/$1
   AC_ARG_WITH([$1[]-dir],
      AS_HELP_STRING([--with-[]$1-dir=PATH],
         [path to installed $1 dicts]))
   AS_IF([test "x$with_[]$1_dir" != "x"],
      [$1[]_dir=$with_[]$1_dir])])

AC_DEFUN([ENCHANT_CHECK_PKG_CONFIG_PROVIDER],
  [ENCHANT_CHECK_PROVIDER_SETUP([$1], [$4])
   AS_IF([test "x$with_[]$1" != xno],
         [PKG_CHECK_MODULES([$2], [m4_default([$3], [$1])],
         [$2[]_CFLAGS="$[]$2[]_CFLAGS -DENCHANT_[]$2[]_DICT_DIR='\"$[]$1_dir\"'"
         with_$1=yes
         build_providers="$build_providers $1"],
         [if test "x$with_$1" != xcheck; then
            AC_MSG_FAILURE([--with-[]$1 was given, but test(s) for $1 failed])
          fi
          with_$1=no])])
   AM_CONDITIONAL(WITH_[]$2, test "x$with_[]$1" = xyes)])

AC_DEFUN([ENCHANT_CHECK_LIB_PROVIDER],
  [ENCHANT_CHECK_PROVIDER_SETUP([$1], [$4])
   AS_IF([test "x$with_[]$1" != xno],
      [_enchant_header=m4_default([$6], [$1[].h])
      AC_CHECK_HEADERS([$_enchant_header], [_header_found=yes], [_header_found=no], [])
      if test "$3" != NOLIB; then
         AC_CHECK_LIB([$1], [$3],,, [$5])
      else
         ac_cv_lib_[]$1[]_[]$3=yes
      fi
      if test "x$_header_found" != xyes -o "x$ac_cv_lib_[]$1[]_[]$3" != xyes; then
         if test "x$with_[]$1" != xcheck; then
            AC_MSG_FAILURE([--with-[]$1 was given, but tests for $1 failed])
         fi
         with_[]$1=no
      else
         with_[]$1=yes
         build_providers="$build_providers $1"
         LIBS="$LIBS $5"
      fi])
   AM_CONDITIONAL(WITH_[]$2, test "x$with_[]$1" = xyes)])


dnl Check for providers
build_providers=

dnl Standard providers
ENCHANT_CHECK_PKG_CONFIG_PROVIDER([hunspell], [HUNSPELL])
ENCHANT_CHECK_PKG_CONFIG_PROVIDER([nuspell], [NUSPELL], [nuspell >= 5.1.0])
ENCHANT_CHECK_LIB_PROVIDER([aspell], [ASPELL], [get_aspell_dict_info_list])
ENCHANT_CHECK_LIB_PROVIDER([hspell], [HSPELL], [hspell_get_dictionary_path],, [-lz])
ENCHANT_CHECK_PKG_CONFIG_PROVIDER([voikko], [VOIKKO], [libvoikko])
dnl FIXME: The test below assumes GCC(-compatible) ObjC++ compiler, but
dnl OBJCXX is set even if no compiler is found.
if test "$ac_cv_objcxx_compiler_gnu" = "yes"; then
  AC_LANG_PUSH([Objective C++])
  ENCHANT_CHECK_LIB_PROVIDER([applespell], [APPLESPELL], [NOLIB],, [-framework Cocoa], [Cocoa/Cocoa.h])
  AC_LANG_POP([Objective C++])
fi
dnl Must call AM_CONDITIONAL outside conditional
AM_CONDITIONAL(WITH_APPLESPELL, test "x$with_applespell" = xyes)

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
providers/Makefile
tests/Makefile
tests/enchant_providers/Makefile
], [],
[ENCHANT_MAJOR_VERSION="$ENCHANT_MAJOR_VERSION"])
AC_OUTPUT

dnl ===========================================================================================

echo "Providers to build:${build_providers}"
if test "x$build_providers" = "x"; then
   AC_MSG_WARN([No spell-checking provider selected!])
fi

if test "x$with_zemberek" = "xyes"; then
   echo "
The Zemberek Turkish spell-checking plugin is enabled. It is known
to cause crashes with WebKit. Use at your own discretion."
fi
if test $RELOCATABLE != yes; then
  AC_MSG_WARN([Building without --enable-relocatable: the tests will not work!])
fi
