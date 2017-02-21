if WITH_HUNSPELL
target_lib = libenchant_hunspell.la
else
target_lib =
endif

AM_CPPFLAGS = -I$(top_srcdir) $(ISYSTEM)$(top_builddir)/lib $(ISYSTEM)$(top_srcdir)/lib -I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(HUNSPELL_CFLAGS) $(WARN_CXXFLAGS) -D_ENCHANT_BUILD=1

hunspell_LTLIBRARIES = $(target_lib)
hunspelldir= $(libdir)/enchant

libenchant_hunspell_lalibdir=$(libdir)/enchant
libenchant_hunspell_la_LIBADD= $(HUNSPELL_LIBS) $(ENCHANT_LIBS) $(top_builddir)/src/libenchant.la
libenchant_hunspell_la_LDFLAGS = -module -avoid-version -no-undefined

libenchant_hunspell_la_SOURCES =	\
	hunspell_checker.cpp

EXTRA_DIST=			\
	libenchant_hunspell.rc

