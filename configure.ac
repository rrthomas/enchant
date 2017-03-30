AC_INIT([enchant],[2.0.0])
AC_CONFIG_SRCDIR(src/enchant.h)
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])

dnl Checks for programs.
AC_PROG_CC
gl_EARLY
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(11)
AC_PROG_INSTALL
AC_HEADER_STDC
LT_INIT
gl_INIT
AC_SUBST([objdir])
LT_SYS_MODULE_EXT
AC_SUBST([shlibext], [$libltdl_cv_shlibext])
gl_RELOCATABLE_LIBRARY
AC_CONFIG_LIBOBJ_DIR([lib])
if test $RELOCATABLE = yes; then
  AC_LIBOBJ([relocatable])
else
  AC_MSG_ERROR([Enchant must be built with --enable-relocatable])
fi
gl_CONFIGMAKE_PREP

libdir_subdir=`echo "$libdir" | sed -e 's|^${exec_prefix}/||'`
AC_SUBST([libdir_subdir])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
Makefile
lib/Makefile
src/Makefile
])
AC_OUTPUT
