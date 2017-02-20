# manywarnings.m4 serial 8
dnl Copyright (C) 2008-2017 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Simon Josefsson

# gl_MANYWARN_COMPLEMENT(OUTVAR, LISTVAR, REMOVEVAR)
# --------------------------------------------------
# Copy LISTVAR to OUTVAR except for the entries in REMOVEVAR.
# Elements separated by whitespace.  In set logic terms, the function
# does OUTVAR = LISTVAR \ REMOVEVAR.
AC_DEFUN([gl_MANYWARN_COMPLEMENT],
[
  gl_warn_set=
  set x $2; shift
  for gl_warn_item
  do
    case " $3 " in
      *" $gl_warn_item "*)
        ;;
      *)
        gl_warn_set="$gl_warn_set $gl_warn_item"
        ;;
    esac
  done
  $1=$gl_warn_set
])

# gl_MANYWARN_ALL_GXX(VARIABLE)
# -----------------------------
# Add all documented G++ warning parameters to variable VARIABLE.
# Note that you need to test them using gl_WARN_ADD if you want to
# make sure your gcc understands it.
AC_DEFUN([gl_MANYWARN_ALL_GXX],
[
  dnl First, check for some issues that only occur when combining multiple
  dnl gcc warning categories.
  AC_REQUIRE([AC_PROG_CXX])
  if test -n "$GXX"; then

    dnl Check if -W -Werror -Wno-missing-field-initializers is supported
    dnl with the current $CXX $CXXFLAGS $CPPFLAGS.
    AC_MSG_CHECKING([whether -Wno-missing-field-initializers is supported])
    AC_CACHE_VAL([gl_cv_cc_nomfi_supported], [
      gl_save_CXXFLAGS="$CXXFLAGS"
      CXXFLAGS="$CXXFLAGS -W -Werror -Wno-missing-field-initializers"
      AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([[]], [[]])],
        [gl_cv_cc_nomfi_supported=yes],
        [gl_cv_cc_nomfi_supported=no])
      CXXFLAGS="$gl_save_CXXFLAGS"])
    AC_MSG_RESULT([$gl_cv_cc_nomfi_supported])

    if test "$gl_cv_cc_nomfi_supported" = yes; then
      dnl Now check whether -Wno-missing-field-initializers is needed
      dnl for the { 0, } construct.
      AC_MSG_CHECKING([whether -Wno-missing-field-initializers is needed])
      AC_CACHE_VAL([gl_cv_cc_nomfi_needed], [
        gl_save_CXXFLAGS="$CXXFLAGS"
        CXXFLAGS="$CXXFLAGS -W -Werror"
        AC_COMPILE_IFELSE(
          [AC_LANG_PROGRAM(
             [[int f (void)
               {
                 typedef struct { int a; int b; } s_t;
                 s_t s1 = { 0, };
                 return s1.b;
               }
             ]],
             [[]])],
          [gl_cv_cc_nomfi_needed=no],
          [gl_cv_cc_nomfi_needed=yes])
        CXXFLAGS="$gl_save_CXXFLAGS"
      ])
      AC_MSG_RESULT([$gl_cv_cc_nomfi_needed])
    fi

    dnl Next, check if -Werror -Wuninitialized is useful with the
    dnl user's choice of $CXXFLAGS; some versions of gcc warn that it
    dnl has no effect if -O is not also used
    AC_MSG_CHECKING([whether -Wuninitialized is supported])
    AC_CACHE_VAL([gl_cv_cc_uninitialized_supported], [
      gl_save_CXXFLAGS="$CXXFLAGS"
      CXXFLAGS="$CXXFLAGS -Werror -Wuninitialized"
      AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([[]], [[]])],
        [gl_cv_cc_uninitialized_supported=yes],
        [gl_cv_cc_uninitialized_supported=no])
      CXXFLAGS="$gl_save_CXXFLAGS"])
    AC_MSG_RESULT([$gl_cv_cc_uninitialized_supported])

  fi

  # List all gcc warning categories.
  # To compare this list to your installed GCC's, run this Bash command:
  #
  # comm -3 \
  #  <(sed -n 's/^  *\(-[^ ]*\) .*/\1/p' manywarnings-cxx.m4 | sort) \
  #  <(gcc --help=warnings | sed -n 's/^  \(-[^ ]*\) .*/\1/p' | sort |
  #      grep -v -x -f <(
  #         awk '/^[^#]/ {print $1}' ../build-aux/g++-warning.spec))

  gl_manywarn_set=
  for gl_manywarn_item in \
    -W \
    -Wabi \
    -Wabi-tag \
    -Waddress \
    -Waggressive-loop-optimizations \
    -Wall \
    -Wattributes \
    -Wbool-compare \
    -Wbuiltin-macro-redefined \
    -Wcast-align \
    -Wchar-subscripts \
    -Wchkp \
    -Wclobbered \
    -Wcomment \
    -Wcomments \
    -Wconditionally-supported \
    -Wconversion-null \
    -Wcoverage-mismatch \
    -Wcpp \
    -Wctor-dtor-privacy \
    -Wdate-time \
    -Wdelete-incomplete \
    -Wdelete-non-virtual-dtor \
    -Wdeprecated \
    -Wdeprecated-declarations \
    -Wdisabled-optimization \
    -Wdiv-by-zero \
    -Wdouble-promotion \
    -Weffc++ \
    -Wempty-body \
    -Wendif-labels \
    -Wenum-compare \
    -Wextra \
    -Wformat-contains-nul \
    -Wformat-extra-args \
    -Wformat-nonliteral \
    -Wformat-security \
    -Wformat-signedness \
    -Wformat-y2k \
    -Wformat-zero-length \
    -Wfree-nonheap-object \
    -Wignored-qualifiers \
    -Winherited-variadic-ctor \
    -Winit-self \
    -Winline \
    -Wint-to-pointer-cast \
    -Winvalid-memory-model \
    -Winvalid-offsetof \
    -Winvalid-pch \
    -Wliteral-suffix \
    -Wlogical-not-parentheses \
    -Wlogical-op \
    -Wmain \
    -Wmaybe-uninitialized \
    -Wmemset-transposed-args \
    -Wmissing-braces \
    -Wmissing-declarations \
    -Wmissing-field-initializers \
    -Wmissing-include-dirs \
    -Wmultichar \
    -Wnarrowing \
    -Wnoexcept \
    -Wnon-template-friend \
    -Wnon-virtual-dtor \
    -Wnonnull \
    -Wodr \
    -Wold-style-cast \
    -Wopenmp-simd \
    -Woverflow \
    -Woverlength-strings \
    -Woverloaded-virtual \
    -Wpacked \
    -Wpacked-bitfield-compat \
    -Wparentheses \
    -Wpmf-conversions \
    -Wpointer-arith \
    -Wpragmas \
    -Wreorder \
    -Wreturn-local-addr \
    -Wreturn-type \
    -Wsequence-point \
    -Wshadow \
    -Wshift-count-negative \
    -Wshift-count-overflow \
    -Wsign-promo \
    -Wsized-deallocation \
    -Wsizeof-array-argument \
    -Wsizeof-pointer-memaccess \
    -Wstack-protector \
    -Wstrict-aliasing \
    -Wstrict-null-sentinel \
    -Wstrict-overflow \
    -Wsuggest-attribute=const \
    -Wsuggest-attribute=format \
    -Wsuggest-attribute=noreturn \
    -Wsuggest-attribute=pure \
    -Wsuggest-final-methods \
    -Wsuggest-final-types \
    -Wsuggest-override \
    -Wswitch \
    -Wswitch-bool \
    -Wswitch-default \
    -Wsync-nand \
    -Wsystem-headers \
    -Wtrampolines \
    -Wtrigraphs \
    -Wtype-limits \
    -Wuninitialized \
    -Wunknown-pragmas \
    -Wunsafe-loop-optimizations \
    -Wunused \
    -Wunused-but-set-parameter \
    -Wunused-but-set-variable \
    -Wunused-function \
    -Wunused-label \
    -Wunused-local-typedefs \
    -Wunused-macros \
    -Wunused-parameter \
    -Wunused-result \
    -Wunused-value \
    -Wunused-variable \
    -Wuseless-cast \
    -Wvarargs \
    -Wvariadic-macros \
    -Wvector-operation-performance \
    -Wvirtual-move-assign \
    -Wvla \
    -Wvolatile-register-var \
    -Wwrite-strings \
    -Wzero-as-null-pointer-constant \
    \
    ; do
    gl_manywarn_set="$gl_manywarn_set $gl_manywarn_item"
  done

  # gcc --help=warnings outputs an unusual form for these options; list
  # them here so that the above 'comm' command doesn't report a false match.
  gl_manywarn_set="$gl_manywarn_set -Warray-bounds=2"
  gl_manywarn_set="$gl_manywarn_set -Wnormalized=nfc"
  gl_manywarn_set="$gl_manywarn_set -Wshift-overflow=2"
  gl_manywarn_set="$gl_manywarn_set -Wunused-const-variable=2"

  # These are needed for older GCC versions.
  if test -n "$GXX"; then
    case `($CXX --version) 2>/dev/null` in
      'g++ (GCC) '[[0-3]].* | \
      'g++ (GCC) '4.[[0-7]].*)
        gl_manywarn_set="$gl_manywarn_set -fdiagnostics-show-option"
        gl_manywarn_set="$gl_manywarn_set -funit-at-a-time"
          ;;
    esac
  fi

  # Disable specific options as needed.
  if test "$gl_cv_cc_nomfi_needed" = yes; then
    gl_manywarn_set="$gl_manywarn_set -Wno-missing-field-initializers"
  fi

  if test "$gl_cv_cc_uninitialized_supported" = no; then
    gl_manywarn_set="$gl_manywarn_set -Wno-uninitialized"
  fi

  $1=$gl_manywarn_set
])

# gl_CXX_WARN_ADD(OPTION, [VARIABLE = WARN_CFLAGS],
#             [PROGRAM = AC_LANG_PROGRAM()])
# ---------------------------------------------
# Adds parameter to WARN_CXXFLAGS if the compiler supports it when
# compiling PROGRAM.  For example, gl_CXX_WARN_ADD([-Wparentheses]).
#
# If VARIABLE is a variable name, AC_SUBST it.
AC_DEFUN([gl_CXX_WARN_ADD],
[AC_REQUIRE([gl_UNKNOWN_WARNINGS_ARE_ERRORS])
gl_COMPILER_OPTION_IF([$1],
  [gl_AS_VAR_APPEND(m4_if([$2], [], [[WARN_CXXFLAGS]], [[$2]]), [" $1"])],
  [],
  [$3])
m4_ifval([$2],
         [AS_LITERAL_IF([$2], [AC_SUBST([$2])])],
         [AC_SUBST([WARN_CXXFLAGS])])dnl
])
