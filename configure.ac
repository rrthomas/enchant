AC_INIT([enchant],[2.0.0])
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

ENCHANT_SONUM=0
AGE=`expr $ENCHANT_MAJOR_VERSION '*' 1000 + $ENCHANT_MINOR_VERSION`
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
AC_PROG_OBJC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_HEADER_STDC
LT_INIT([win32-dll])
LT_LANG([Windows Resource])
gl_INIT
ENCHANT_LT_OBJDIR=$lt_cv_objdir
AC_SUBST(ENCHANT_LT_OBJDIR)


PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.6 gmodule-2.0])

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
  nw="$nw -Wundef"            # All compiler preprocessors support #if UNDEF
  nw="$nw -Wtraditional"      # All compilers nowadays support ANSI C
  nw="$nw -Wdeclaration-after-statement" # We require C99.
  nw="$nw -Wstrict-overflow"  # Use a lower level (see below).
  nw="$nw -Wconversion"       # These warnings usually don’t point to mistakes.
  nw="$nw -Wsign-conversion"  # Likewise.

  gl_MANYWARN_ALL_GCC([warnings])

  dnl Enable all GCC warnings not in this list.
  gl_MANYWARN_COMPLEMENT([warnings], [$warnings], [$nw])
  for w in $warnings; do
    gl_WARN_ADD([$w])
  done

  gl_MANYWARN_ALL_GXX([cxx_warnings])

  dnl Enable all G++ warnings not in this list.
  gl_MANYWARN_COMPLEMENT([cxx_warnings], [$cxx_warnings], [$nw])
  for w in $cxx_warnings; do
    gl_CXX_WARN_ADD([$w])
  done

  dnl Add an extra warning
  gl_WARN_ADD([-Wstrict-overflow=1])
  dnl Add some more safety measures
  gl_WARN_ADD([-D_FORTIFY_SOURCE=2])
  dnl Remove a warning being promoted to error: we trigger this and can't turn it off with pragmas.
  gl_WARN_ADD([-Wno-error=format-security])

  # When compiling with GCC, prefer -isystem to -I when including system
  # include files, to avoid generating useless diagnostics for the files.
  ISYSTEM='-isystem '
else
  ISYSTEM='-I'
fi
AC_SUBST([ISYSTEM])

ENCHANT_CFLAGS=$GLIB_CFLAGS
ENCHANT_LIBS=$GLIB_LIBS
if test "x$prefix" != "xNONE"; then
	ENCHANT_CFLAGS="$ENCHANT_CFLAGS -DENCHANT_PREFIX_DIR='\"$prefix\"'"
fi

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


dnl ===========================================================================
dnl Travis runs Ubuntu 14.04 LTS, which doesn't include the following macro
dnl added to pkg-config in January 2013.
dnl ===========================================================================
m4_ifndef([PKG_CHECK_VAR], [
AC_DEFUN([PKG_CHECK_VAR],
  [AC_REQUIRE([PKG_PROG_PKG_CONFIG])dnl
     AC_ARG_VAR([$1], [value of $3 for $2, overriding pkg-config])dnl
     _PKG_CONFIG([$1], [variable="][$3]["], [$2])
     AS_VAR_COPY([$1], [pkg_cv_][$1])
     AS_VAR_IF([$1], [""], [$5], [$4])dnl
  ])# PKG_CHECK_VAR
])

dnl ===========================================================================
dnl Very hackish.
dnl Ubuntu / Debian ship older version of unittest++ with their own pkg-config
dnl and include
dnl ===========================================================================
PKG_CHECK_EXISTS([UnitTest++],
    [PKG_CHECK_MODULES(UNITTESTPP, [UnitTest++])
dnl and because of the above, we need to tweak the include until we stop
dnl supporting this old version.
        PKG_CHECK_VAR(UNITTESTPP_INCLUDE, [UnitTest++], [includedir])
        if test "x$UNITTESTPP_INCLUDE" != "x"; then
            UNITTESTPP_CFLAGS="$UNITTESTPP_CFLAGS -I$UNITTESTPP_INCLUDE/UnitTest++"
        fi
    ],
    [PKG_CHECK_EXISTS([unittest++], [
        PKG_CHECK_MODULES(UNITTESTPP, [unittest++])
            dnl Arch ships a broken custom pkgconfig file, try to fix includedir
            dnl https://bugs.archlinux.org/task/44516
            AC_LANG_PUSH([C++])
            CACHED_CXXFLAGS="$CXXFLAGS"
            CXXFLAGS="$CXXFLAGS $UNITTESTPP_CFLAGS"
            AC_CHECK_HEADERS([UnitTest++.h], [], [
                PKG_CHECK_VAR(UNITTESTPP_INCLUDE, [unittest++], [includedir])
                UNITTESTPP_CFLAGS="$UNITTESTPP_CFLAGS -I$UNITTESTPP_INCLUDE/UnitTest++"

                CXXFLAGS="$CACHED_CXXFLAGS $UNITTESTPP_CFLAGS"
                unset ac_cv_header_UnitTestpp_h
                AC_CHECK_HEADERS([UnitTest++.h], [], [
                    AC_MSG_WARN([Failed to include UnitTest++.h])
                ])
            ])
            CXXFLAGS="$CACHED_CXXFLAGS"
            AC_LANG_POP
        ]
    )]
)
AC_SUBST(UNITTESTPP_CFLAGS)
AC_SUBST(UNITTESTPP_LIBS)


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
         build_providers+=" $1"],
         [if test "x$with_$1" != xcheck; then
            AC_MSG_FAILURE([--with-[]$1 was given, but test(s) for $1 failed])
          fi
          with_$1=no])])
   AM_CONDITIONAL(WITH_[]$2, test "x$with_[]$1" = xyes)])

AC_DEFUN([ENCHANT_CHECK_LIB_PROVIDER],
  [ENCHANT_CHECK_PROVIDER_SETUP([$1], [$4])
   AS_IF([test "x$with_[]$1" != xno],
      [AC_CHECK_HEADERS([$1[].h])
      AC_CHECK_LIB([$1], [$3])

      if test "x$ac_cv_header_[]$1_h" != xyes -o "x$ac_cv_lib_[]$1[]_[]$3" != xyes; then
         if test "x$with_[]$1" != xcheck; then
            AC_MSG_FAILURE([--with-[]$1 was given, but tests for $1 failed])
         fi
         with_[]$1=no
      else
         with_[]$1=yes
         build_providers+=" $1"
      fi])
   AM_CONDITIONAL(WITH_[]$2, test "x$with_[]$1" = xyes)])


dnl Check for providers
build_providers=

dnl Standard providers
ENCHANT_CHECK_PKG_CONFIG_PROVIDER([hunspell], [HUNSPELL])
ENCHANT_CHECK_LIB_PROVIDER([aspell], [ASPELL], [get_aspell_dict_info_list])
ENCHANT_CHECK_LIB_PROVIDER([hspell], [HSPELL], [hspell_get_dictionary_path])
ENCHANT_CHECK_PKG_CONFIG_PROVIDER([voikko], [VOIKKO], [libvoikko])

dnl Experimental/deprecated providers
ENCHANT_CHECK_PKG_CONFIG_PROVIDER([uspell], [USPELL], , [no])
ENCHANT_CHECK_PKG_CONFIG_PROVIDER([zemberek], [ZEMBEREK], [dbus-glib-1 >= 0.62], [no])

dnl Apple Spell
ENCHANT_CHECK_PROVIDER_SETUP([applespell])
AS_IF([test "x$with_applespell" != xno],
   [AC_MSG_CHECKING([for applespell])
    case ${host_os} in
        *darwin*)
           APPLESPELL_CFLAGS=" -xobjective-c"
           APPLESPELL_LIBS=" -lobjc"
           APPLESPELL_LDFLAGS=" -framework Cocoa"
           with_applespell=yes
           build_providers+=" applespell"
        ;;
        *)
           if test "x$with_applespell" != xcheck; then
             AC_MSG_FAILURE([--with-applespell was given, but not building for macOS])
           fi
           with_applespell=no
        ;;
    esac
    AC_MSG_RESULT([$with_applespell])
   ])
AC_SUBST(APPLESPELL_CFLAGS)
AC_SUBST(APPLESPELL_LIBS)
AC_SUBST(APPLESPELL_LDFLAGS)

AM_CONDITIONAL(WITH_APPLESPELL, test "x$with_applespell" = xyes)

dnl =======================================================================================

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
Makefile
enchant.pc
lib/Makefile
src/libenchant.rc
src/Makefile
src/aspell/Makefile
src/uspell/Makefile
src/hunspell/Makefile
src/hspell/Makefile
src/applespell/Makefile
src/voikko/Makefile
src/zemberek/Makefile
tests/Makefile
unittests/Makefile
unittests/enchant_providers/Makefile
unittests/mock_provider/Makefile
])
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
