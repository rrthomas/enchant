if WITH_MYSPELL
target_lib = libenchant_myspell.la
else
target_lib =
endif

EXTRA_DIST=license.readme

INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(MYSPELL_CFLAGS)

myspell_LTLIBRARIES = $(target_lib)
myspelldir= $(libdir)/enchant

libenchant_myspell_lalibdir=$(libdir)/enchant
libenchant_myspell_la_LIBADD= $(ENCHANT_LIBS) $(top_builddir)/src/libenchant.la
libenchant_myspell_la_LDFLAGS = -version-info $(VERSION_INFO) -no-undefined
libenchant_myspell_la_SOURCES =	\
	affentry.cxx		\
	affentry.hxx		\
	affixmgr.cxx		\
	affixmgr.hxx		\
	atypes.hxx		\
	baseaffix.hxx		\
	csutil.cxx		\
	csutil.hxx		\
	hashmgr.cxx		\
	hashmgr.hxx		\
	htypes.hxx		\
	myspell_checker.cpp	\
	myspell_checker.h	\
	myspell.cxx		\
	myspell.hxx		\
	suggestmgr.cxx		\
	suggestmgr.hxx
