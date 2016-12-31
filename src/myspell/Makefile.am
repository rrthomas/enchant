if WITH_MYSPELL
target_lib = libenchant_myspell.la
else
target_lib =
endif

AM_CPPFLAGS=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(MYSPELL_CFLAGS) $(CXX_WARN_CFLAGS) -D_ENCHANT_BUILD=1

myspell_LTLIBRARIES = $(target_lib)
myspelldir= $(libdir)/enchant

libenchant_myspell_lalibdir=$(libdir)/enchant
libenchant_myspell_la_LIBADD= $(MYSPELL_LIBS) $(ENCHANT_LIBS) $(top_builddir)/src/libenchant.la
libenchant_myspell_la_LDFLAGS = -module -avoid-version -no-undefined

libenchant_myspell_la_SOURCES =	\
	myspell_checker.cpp

EXTRA_DIST=			\
	libenchant_myspell.rc

