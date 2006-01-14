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

libenchant_myspell_la_SOURCES =	\
	affentry.hxx		\
	affixmgr.hxx		\
	atypes.hxx		\
	baseaffix.hxx		\
	csutil.hxx		\
	dictmgr.hxx		\
	hashmgr.hxx		\
	htypes.hxx		\
	hunspell.hxx		\
	langnum.hxx		\
	suggestmgr.hxx		\
	affentry.cxx		\
	affixmgr.cxx		\
	csutil.cxx		\
	dictmgr.cxx		\
	hashmgr.cxx		\
	hunspell.cxx		\
	suggestmgr.cxx		\
	myspell_checker.cpp

EXTRA_DIST=			\
	license.readme		\
	utf_info.cxx		\
	README			\
	license.hunspell	\
	license.myspell		\
	license.readme		\
	hunspell.dsp

