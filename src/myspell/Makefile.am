if WITH_MYSPELL
target_lib = libenchant_myspell.la
else
target_lib =
endif

INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(MYSPELL_CFLAGS) -D_ENCHANT_BUILD=1

myspell_LTLIBRARIES = $(target_lib)
myspelldir= $(libdir)/enchant

libenchant_myspell_lalibdir=$(libdir)/enchant
libenchant_myspell_la_LIBADD= $(MYSPELL_LIBS) $(ENCHANT_LIBS) $(top_builddir)/src/libenchant.la
libenchant_myspell_la_LDFLAGS = -version-info $(VERSION_INFO) -no-undefined

if WITH_SYSTEM_MYSPELL
libenchant_myspell_la_SOURCES =	\
	myspell_checker.cpp
else
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
	myspell.cxx		\
	enchant_myspell.hxx	\
	suggestmgr.cxx		\
	suggestmgr.hxx		\
	myspell_checker.cpp
endif

EXTRA_DIST=			\
	license.readme		\
	affentry.hxx		\
	affixmgr.hxx		\
	atypes.hxx		\
	baseaffix.hxx		\
	csutil.hxx		\
	hashmgr.hxx		\
	htypes.hxx		\
	enchant_myspell.hxx	\
	suggestmgr.hxx		\
	affentry.cxx		\
	affixmgr.cxx		\
	csutil.cxx		\
	hashmgr.cxx		\
	myspell.cxx		\
	suggestmgr.cxx
