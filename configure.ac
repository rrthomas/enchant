AC_INIT([enchant],[1.6.1])
AC_PACKAGE_VERSION
AC_CONFIG_SRCDIR(src/enchant.h)
AM_INIT_AUTOMAKE([no-define subdir-objects])
AC_CANONICAL_HOST
AM_MAINTAINER_MODE
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

dnl Version info for libraries = CURRENT:REVISION:AGE
MAJOR_VERSION_PLUS_MINOR_VERSION=`expr $ENCHANT_MAJOR_VERSION + $ENCHANT_MINOR_VERSION`
VERSION_INFO=$MAJOR_VERSION_PLUS_MINOR_VERSION:$ENCHANT_MICRO_VERSION:$ENCHANT_MINOR_VERSION

AC_SUBST(VERSION_INFO)
AC_SUBST(ENCHANT_MAJOR_VERSION)
AC_SUBST(ENCHANT_MINOR_VERSION)
AC_SUBST(ENCHANT_MICRO_VERSION)

dnl Checks for programs.
AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(11)
AC_PROG_OBJC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_HEADER_STDC
LT_INIT([win32-dll])
ENCHANT_LT_OBJDIR=$lt_cv_objdir
AC_SUBST(ENCHANT_LT_OBJDIR)
gt_TYPE_SSIZE_T

AC_CHECK_FUNCS(flock lockf)


PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(ENCHANT, [glib-2.0 >= 2.6 gmodule-2.0])

dnl ===========================================================================
dnl check compiler flags
AC_DEFUN([ENCHANT_CC_TRY_FLAG], [
  AC_MSG_CHECKING([whether $CC supports $1])

  enchant_save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS $1"

  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ ]])], [enchant_cc_flag=yes], [enchant_cc_flag=no])
  CFLAGS="$enchant_save_CFLAGS"

  if test "x$enchant_cc_flag" = "xyes"; then
    ifelse([$2], , :, [$2])
  else
    ifelse([$3], , :, [$3])
  fi
  AC_MSG_RESULT([$enchant_cc_flag])
])

dnl check compiler flags
AC_DEFUN([ENCHANT_CXX_TRY_FLAG], [
  AC_MSG_CHECKING([whether $CXX supports $1])

  AC_LANG_PUSH(C++)
  enchant_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS $1"

  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ ]])], [enchant_cxx_flag=yes], [enchant_cxx_flag=no])
  CXXFLAGS="$enchant_save_CXXFLAGS"

  if test "x$enchant_cxx_flag" = "xyes"; then
    ifelse([$2], , :, [$2])
  else
    ifelse([$3], , :, [$3])
  fi
  AC_MSG_RESULT([$enchant_cxx_flag])
  AC_LANG_POP(C++)
])

dnl Use lots of warning flags with gcc and compatible compilers

dnl Note: if you change the following variable, the cache is automatically
dnl skipped and all flags rechecked.  So there's no need to do anything
dnl else.  If for any reason you need to force a recheck, just change
dnl MAYBE_WARN in an ignorable way (like adding whitespace)

CC_MAYBE_WARN="-Wall -Wextra \
-Wsign-compare -Werror-implicit-function-declaration \
-Wpointer-arith -Wwrite-strings -Wstrict-prototypes \
-Wmissing-prototypes -Wmissing-declarations -Wnested-externs \
-Wpacked -Wswitch-enum -Wmissing-format-attribute \
-Wstrict-aliasing=2 -Winit-self -Wunsafe-loop-optimizations \
-Wdeclaration-after-statement -Wold-style-definition \
-Wno-missing-field-initializers -Wno-unused-parameter \
-Wno-attributes -Wno-long-long -Winline -fno-strict-aliasing"

# invalidate cached value if MAYBE_WARN has changed
if test "x$enchant_cv_warn_maybe" != "x$CC_MAYBE_WARN"; then
	unset enchant_cv_warn_cflags
fi
AC_CACHE_CHECK([for supported C warning flags], enchant_cv_warn_cflags, [
	echo
	CC_WARN_CFLAGS=""

	# Some warning options are not supported by all versions of
	# gcc, so test all desired options against the current
	# compiler.
	#
	# Note that there are some order dependencies
	# here. Specifically, an option that disables a warning will
	# have no net effect if a later option then enables that
	# warnings, (perhaps implicitly). So we put some grouped
	# options (-Wall and -Wextra) up front and the -Wno options
	# last.

	for W in $CC_MAYBE_WARN; do
		ENCHANT_CC_TRY_FLAG([$W], [CC_WARN_CFLAGS="$CC_WARN_CFLAGS $W"])
	done

	enchant_cv_warn_cflags=$CC_WARN_CFLAGS
	enchant_cv_warn_maybe=$CC_MAYBE_WARN

	AC_MSG_CHECKING([which C warning flags were supported])])
CC_WARN_CFLAGS="$enchant_cv_warn_cflags"
AC_SUBST(CC_WARN_CFLAGS)

CXX_MAYBE_WARN="-Wall -Wextra \
-Wsign-compare \
-Wpointer-arith -Wwrite-strings \
-Wpacked -Wswitch-enum -Wmissing-format-attribute \
-Wstrict-aliasing=2 -Winit-self -Wunsafe-loop-optimizations \
-Wno-missing-field-initializers -Wno-unused-parameter \
-Wno-attributes -Wno-long-long -Winline -fno-strict-aliasing"

# invalidate cached value if MAYBE_WARN has changed
if test "x$enchant_cxx_cv_warn_maybe" != "x$CXX_MAYBE_WARN"; then
	unset enchant_cxx_cv_warn_cflags
fi
AC_CACHE_CHECK([for supported C++ warning flags], enchant_cxx_cv_warn_cflags, [
	echo
	CXX_WARN_CFLAGS=""

	# Some warning options are not supported by all versions of
	# gcc, so test all desired options against the current
	# compiler.
	#
	# Note that there are some order dependencies
	# here. Specifically, an option that disables a warning will
	# have no net effect if a later option then enables that
	# warnings, (perhaps implicitly). So we put some grouped
	# options (-Wall and -Wextra) up front and the -Wno options
	# last.

	for W in $CXX_MAYBE_WARN; do
		ENCHANT_CXX_TRY_FLAG([$W], [CXX_WARN_CFLAGS="$CXX_WARN_CFLAGS $W"])
	done

	enchant_cxx_cv_warn_cflags=$CXX_WARN_CFLAGS
	enchant_cxx_cv_warn_maybe=$CXX_MAYBE_WARN

	AC_MSG_CHECKING([which C++ warning flags were supported])])
CXX_WARN_CFLAGS="$enchant_cxx_cv_warn_cflags"
AC_SUBST(CXX_WARN_CFLAGS)

if test "x$prefix" != "xNONE"; then
	ENCHANT_CFLAGS="$ENCHANT_CFLAGS -DENCHANT_PREFIX_DIR='\"$prefix\"'"
fi

AC_SUBST(ENCHANT_CFLAGS)
AC_SUBST(ENCHANT_LIBS)

AC_MSG_CHECKING([for native Win32])
case "$target" in
  *-*-mingw*)
    native_win32=yes
    SOCKET_LIBS='-lws2_32 -ldnsapi'
    ENCHANT_WIN32_RESOURCE=enchant-win32res.lo
    ;;
  *)
    native_win32=no
    SOCKET_LIBS=''
    ENCHANT_WIN32_RESOURCE=
    ;;
esac
AC_MSG_RESULT([$native_win32])
AM_CONDITIONAL(OS_WIN32, test "x$native_win32" = "xyes")
AC_SUBST([ENCHANT_WIN32_RESOURCE])

AC_MSG_CHECKING([for OS X])
case ${host_os} in
    *darwin*)
    check_applespell=yes
    ;;
    *)
    check_applespell=no
    ;;
esac
AC_MSG_RESULT([$check_applespell])

AC_SUBST(SOCKET_LIBS)

# Courtesy of Glib: Ensure MSVC-compatible struct packing convention 
# is used when compiling for Win32 with gcc.
# What flag to depends on gcc version: gcc3 uses "-mms-bitfields", while
# gcc2 uses "-fnative-struct".
if test x"$native_win32" = xyes; then
  if test x"$GCC" = xyes; then
    msnative_struct=''
    AC_MSG_CHECKING([how to get MSVC-compatible struct packing])
    if test -z "$ac_cv_prog_CC"; then
      our_gcc="$CC"
    else
      our_gcc="$ac_cv_prog_CC"
    fi
    case `$our_gcc --version | sed -e 's,\..*,.,' -e q` in
      2.)
	if $our_gcc -v --help 2>/dev/null | grep fnative-struct >/dev/null; then
	  msnative_struct='-fnative-struct'
	fi
	;;
      *)
	if $our_gcc -v --help 2>/dev/null | grep ms-bitfields >/dev/null; then
	  msnative_struct='-mms-bitfields'
	fi
	;;
    esac
    if test x"$msnative_struct" = x ; then
      AC_MSG_RESULT([no way])
      AC_MSG_WARN([produced libraries might be incompatible with MSVC-compiled code])
    else
      CFLAGS="$CFLAGS $msnative_struct"
      CXXFLAGS="$CXXFLAGS $msnative_struct"
      AC_MSG_RESULT([${msnative_struct}])
    fi
  fi
fi 

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
dnl Travis run Ubuntu 14.04 LTS which doesn't include that macro
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
dnl very hackish.
dnl Unbuntu / Debian ship older version of unittest++ with their own pkg-config
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
    [PKG_CHECK_EXISTS([unittest++],
        [PKG_CHECK_MODULES(UNITTESTPP, [unittest++])]
    )]
)
AC_SUBST(UNITTESTPP_CFLAGS)
AC_SUBST(UNITTESTPP_LIBS)

build_ispell=yes

AC_ARG_ENABLE(ispell, AS_HELP_STRING([--disable-ispell],[enable the ispell backend @<:@default=auto@:>@]), build_ispell="$enableval", build_ispell=yes)

AM_CONDITIONAL(WITH_ISPELL, test "x$build_ispell" = "xyes")

ispell_dir=${datadir}/enchant/ispell
AC_ARG_WITH(ispell-dir, AS_HELP_STRING([--with-ispell-dir=PATH],[path to installed ispell dicts]))

if test "x$with_ispell_dir" != "x" ; then
   ispell_dir=$with_ispell_dir
fi

ISPELL_CFLAGS="-DENCHANT_ISPELL_DICT_DIR='\"$ispell_dir\"'"
AC_SUBST(ISPELL_CFLAGS)

build_myspell=yes

AC_ARG_ENABLE(myspell, AS_HELP_STRING([--disable-myspell],[enable the myspell backend @<:@default=auto@:>@]), build_myspell="$enableval", build_myspell=yes)

AM_CONDITIONAL(WITH_MYSPELL, test "x$build_myspell" = "xyes")

myspell_dir=${datadir}/myspell/dicts
AC_ARG_WITH(myspell-dir, AS_HELP_STRING([--with-myspell-dir=PATH],[path to installed myspell dicts]))

if test "x$with_myspell_dir" != "x" ; then
   myspell_dir=$with_myspell_dir
fi

PKG_CHECK_MODULES(MYSPELL, [hunspell])
AC_SUBST(MYSPELL_CFLAGS)
AC_SUBST(MYSPELL_LIBS)
MYSPELL_CFLAGS="$MYSPELL_CFLAGS -DENCHANT_MYSPELL_DICT_DIR='\"$myspell_dir\"'"

check_aspell=yes
build_aspell=no

AC_ARG_ENABLE(aspell, AS_HELP_STRING([--disable-aspell],[enable the aspell backend @<:@default=auto@:>@]), check_aspell="$enableval", check_aspell=yes)

AC_ARG_WITH(aspell-prefix, AS_HELP_STRING([--with-aspell-prefix=DIR],[specify under which prefix aspell is installed]), aspell_prefix="$withval", )

if test "x$check_aspell" != "xno"; then
   saved_LDFLAGS=$LDFLAGS

   ASPELL_CFLAGS=
   if test "x$aspell_prefix" != "x"; then
       LDFLAGS="-L$aspell_prefix/lib "$LDFLAGS
       ASPELL_INC="-I$aspell_prefix/include"
       ASPELL_LIBS="-L$aspell_prefix/lib -laspell"
   else
       aspell_prefix=${prefix}
       if test "x$prefix" = "xNONE"; then
	  aspell_prefix="/usr/local"
       fi

       AC_CHECK_PROG(HAVE_ASPELL, aspell, yes, no)
       if test "x$HAVE_ASPELL" = "xyes"; then
	  AC_MSG_CHECKING([For Aspell >= 0.50.0])
	  aspell_major=`aspell -v | awk -F. '{print $4;}' | awk -F")" '{print $1;}'`
	  if test "$aspell_major" -ge "50"; then
	      aspell_prefix=`aspell config prefix`
              LDFLAGS="-L${aspell_prefix}/lib "$LDFLAGS
	      ASPELL_INC="-I${aspell_prefix}/include"
	      ASPELL_LIBS="-L${aspell_prefix}/lib -laspell"
	      AC_MSG_RESULT([yes])
	  else
	      AC_MSG_RESULT([unknown])
	  fi
      fi

      if test "x$HAVE_ASPELL" != "xyes"; then
	      AC_MSG_WARN([Didn't find aspell >= 0.50.0 and no explicit path aspell specified. Guessing at $aspell_prefix])
	      ASPELL_LIBS="-L$aspell_prefix/lib -laspell"
      fi
   fi

   AC_MSG_CHECKING([For aspell headers in $aspell_prefix/include])
   if test -f "$aspell_prefix/include/pspell/pspell.h"; then
       AC_MSG_RESULT([yes (pspell)])

       AC_CHECK_LIB(aspell,new_aspell_config,build_aspell=yes,
          [AC_CHECK_LIB(pspell, new_pspell_config, build_aspell=yes, AC_MSG_WARN([Enchant building without the aspell library]))]
         ,)
       AC_CHECK_LIB(aspell,get_aspell_dict_info_list,ASPELL_CFLAGS="-DASPELL_0_50_0=1",)

       ASPELL_CFLAGS+=" -DHAVE_PSPELL_H"
   elif test -f "$aspell_prefix/include/aspell.h"; then
       AC_MSG_RESULT([yes (aspell)])

       AC_CHECK_LIB(aspell,new_aspell_config,build_aspell=yes,
          [AC_CHECK_LIB(pspell, new_pspell_config, build_aspell=yes, AC_MSG_WARN([Enchant building without the aspell library]))]
         ,)
       AC_CHECK_LIB(aspell,get_aspell_dict_info_list,ASPELL_CFLAGS="-DASPELL_0_50_0=1",)
   else
       AC_MSG_RESULT([no])
       if test "x$HAVE_ASPELL" = "xyes"; then
           AC_MSG_WARN([Didn't find pspell (aspell) headers])
       fi
   fi
   LDFLAGS=$saved_LDFLAGS
   
   AC_SUBST(ASPELL_INC)
   AC_SUBST(ASPELL_LIBS)
   AC_SUBST(ASPELL_CFLAGS)
fi

AM_CONDITIONAL(WITH_ASPELL, test "$build_aspell" = yes)

check_voikko=yes
build_voikko=no

AC_ARG_ENABLE(voikko, AS_HELP_STRING([--disable-voikko],[enable the voikko backend @<:@default=auto@:>@]), check_voikko="$enableval", check_voikko=yes)

AC_ARG_WITH(voikko-prefix, AS_HELP_STRING([--with-voikko-prefix=DIR],[specify under which prefix voikko is installed]), voikko_prefix="$withval", )

if test "x$check_voikko" != "xno"; then
   saved_LDFLAGS=$LDFLAGS

   VOIKKO_INC=
   VOIKKO_LIBS="-lvoikko"
   if test "x$voikko_prefix" != "x"; then
       LDFLAGS="-L$voikko_prefix/lib "$LDFLAGS
       VOIKKO_INC="-I$voikko_prefix/include"
       VOIKKO_LIBS="-L$voikko_prefix/lib "$VOIKKO_LIBS
   fi

   AC_CHECK_LIB(voikko,voikko_init,build_voikko=yes)

   LDFLAGS=$saved_LDFLAGS

   AC_SUBST(VOIKKO_INC)
   AC_SUBST(VOIKKO_LIBS)
fi

AM_CONDITIONAL(WITH_VOIKKO, test "$build_voikko" = yes)

build_uspell=no

check_uspell=yes
AC_ARG_ENABLE(uspell, AS_HELP_STRING([--disable-uspell],[enable the uspell backend @<:@default=auto@:>@]), check_uspell="$enableval", check_uspell=yes)

if test "x$check_uspell" != "xno"; then
   PKG_CHECK_MODULES(USPELL, [libuspell >= 1.1.0], build_uspell=yes, build_uspell=no)
fi

uspell_dir=${datadir}/uspell
AC_ARG_WITH(uspell-dir, AS_HELP_STRING([--with-uspell-dir=PATH],[path to installed uspell dicts]))

if test "x$with_uspell_dir" != "x" ; then
   uspell_dir=$with_uspell_dir
fi

USPELL_CFLAGS="$USPELL_CFLAGS -DENCHANT_USPELL_DICT_DIR='\"$uspell_dir\"'"
AC_SUBST(USPELL_CFLAGS)
AC_SUBST(USPELL_LIBS)

AM_CONDITIONAL(WITH_USPELL, test "$build_uspell" = yes)

build_hspell=no
check_hspell=yes
AC_ARG_ENABLE(hspell, AS_HELP_STRING([--disable-hspell],[enable the hspell backend @<:@default=auto@:>@]), check_hspell="$enableval", check_hspell=yes)

AC_ARG_WITH(hspell-prefix, AS_HELP_STRING([--with-hspell-prefix=DIR],[specify under which prefix hspell is installed]), hspell_prefix="$withval", )

dnl change to pkg-config when hspell provides a hspell.pc file

if test "x$check_hspell" != "xno"; then
   saved_LDFLAGS=$LDFLAGS

   HSPELL_CFLAGS=
   HSPELL_LIBS=" -lhspell -lz"

   if test "x$hspell_prefix" != "x"; then       
       LDFLAGS="-L$hspell_prefix/lib "$LDFLAGS
       HSPELL_CFLAGS="-I$hspell_prefix/include"
       HSPELL_LIBS="-L$hspell_prefix/lib "$HSPELL_LIBS
   fi

   AC_CHECK_LIB(hspell, hspell_get_dictionary_path, build_hspell=yes, build_hspell=no, -lz)
   LDFLAGS=$saved_LDFLAGS

   AC_SUBST(HSPELL_CFLAGS)
   AC_SUBST(HSPELL_LIBS)
fi

AM_CONDITIONAL(WITH_HSPELL, test "$build_hspell" = yes)

build_zemberek=no

AC_ARG_ENABLE(zemberek, AS_HELP_STRING([--disable-zemberek],[enable the experimental zemberek (turkish) backend @<:@default=auto@:>@]), build_zemberek="$enableval", build_zemberek=no)

PKG_CHECK_MODULES(ZEMBEREK, [dbus-glib-1 >= 0.62], build_zemberek=yes, build_zemberek=no)

AC_SUBST(ZEMBEREK_CFLAGS)
AC_SUBST(ZEMBEREK_LIBS)

AM_CONDITIONAL(WITH_ZEMBEREK, test "x$build_zemberek" = "xyes")

zemberek_dir=${datadir}/enchant/zemberek

APPLESPELL_CFLAGS=""
APPLESPELL_LIBS=""
APPLESPELL_LDFLAGS=""

build_applespell=no
if test "x$check_applespell" = "xyes"; then

    AC_ARG_ENABLE(applespell, AS_HELP_STRING([--disable-applespell],[enable the applespell backend @<:@default=no@:>@]), build_applespell="$enableval", build_applespell=no)

    if test "x$build_applespell" != "xno"; then
        APPLESPELL_CFLAGS+=" -DXP_TARGET_COCOA -xobjective-c"
        APPLESPELL_LIBS+=" -lobjc"
        APPLESPELL_LDFLAGS+=" -framework Cocoa"
    fi

fi

AC_SUBST(APPLESPELL_CFLAGS)
AC_SUBST(APPLESPELL_LIBS)
AC_SUBST(APPLESPELL_LDFLAGS)

AM_CONDITIONAL(WITH_APPLESPELL, test "x$build_applespell" = "xyes")

dnl =======================================================================================

AC_CONFIG_FILES([
Makefile
enchant.pc
enchant-uninstalled.pc
enchant.spec
src/libenchant.rc
src/Makefile
src/aspell/Makefile
src/ispell/Makefile
src/uspell/Makefile
src/myspell/Makefile
src/hspell/Makefile
src/applespell/Makefile
src/voikko/Makefile
src/zemberek/Makefile
tests/Makefile
tests/ispell
unittests/Makefile
unittests/enchant_providers/Makefile
unittests/mock_provider/Makefile
doc/Makefile
data/Makefile
])
AC_OUTPUT

dnl ===========================================================================================

relocatable_library="no"
if test "x$native_win32" = "xyes"; then
   relocatable_library="yes"
fi

echo "
$PACKAGE-$VERSION

    prefix:                                 ${prefix}
    compiler:                               ${CC}

    Build Aspell backend:                   ${build_aspell}
    Build Ispell backend:                   ${build_ispell}
    Build Uspell backend:                   ${build_uspell}
    Build Hspell backend:                   ${build_hspell}
    Build Myspell/Hunspell backend:         ${build_myspell}
    Build Voikko backend (Linux only):      ${build_voikko}
    Build Zemberek backend:                 ${build_zemberek}
    Build Apple Spell backend (OS X only):  ${build_applespell}
    Build a relocatable library:            ${relocatable_library}"

if test "x$build_zemberek" = "xyes"; then
   echo "
The Zemberek Turkish spell-checking plugin is enabled. It is known 
to cause crashes with WebKit. Use at your own discretion."
fi
