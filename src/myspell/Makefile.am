if WITH_MYSPELL
target_lib = libenchant_myspell.la
else
target_lib =
endif

# copied from hunspell 1.2.2
COPIED_MYSPELL_FILES=		\
	affentry.cxx		\
	affentry.hxx		\
	affixmgr.cxx		\
	affixmgr.hxx		\
	atypes.hxx		\
	baseaffix.hxx		\
	csutil.cxx		\
	csutil.hxx		\
	dictmgr.cxx		\
	dictmgr.hxx		\
	filemgr.cxx		\
	filemgr.hxx		\
	hashmgr.cxx		\
	hashmgr.hxx		\
	htypes.hxx		\
	hunspell.cxx		\
	hunspell.h		\
	hunspell.hxx		\
	hunzip.cxx		\
	hunzip.hxx		\
	langnum.hxx		\
	phonet.cxx		\
	phonet.hxx		\
	suggestmgr.cxx		\
	suggestmgr.hxx		\
	utf_info.cxx		\
	w_char.hxx

if WITH_SYSTEM_MYSPELL
EXTRA_MYSPELL_FILES=
EXTRA_MYSPELL_CFLAGS=-DWITH_SYSTEM_MYSPELL
else
EXTRA_MYSPELL_FILES=$(COPIED_MYSPELL_FILES)
EXTRA_MYSPELL_CFLAGS=
endif

INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(MYSPELL_CFLAGS) $(CXX_WARN_CFLAGS) $(EXTRA_MYSPELL_CFLAGS) -D_ENCHANT_BUILD=1

myspell_LTLIBRARIES = $(target_lib)
myspelldir= $(libdir)/enchant

libenchant_myspell_lalibdir=$(libdir)/enchant
libenchant_myspell_la_LIBADD= $(MYSPELL_LIBS) $(ENCHANT_LIBS) $(top_builddir)/src/libenchant.la
libenchant_myspell_la_LDFLAGS = -module -avoid-version -no-undefined

libenchant_myspell_la_SOURCES =	\
	$(EXTRA_MYSPELL_FILES)	\
	myspell_checker.cpp

EXTRA_DIST=			\
	$(COPIED_MYSPELL_FILES)	\
	license.readme		\
	utf_info.cxx		\
	license.hunspell	\
	license.myspell		\
	license.readme		\
	hunspell.dsp

