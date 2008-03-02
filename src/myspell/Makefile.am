if WITH_MYSPELL
target_lib = libenchant_myspell.la
else
target_lib =
endif

# copied from hunspell 1.2.1
COPIED_MYSPELL_FILES=		\
	affentry.hxx		\
	affixmgr.hxx		\
	atypes.hxx		\
	baseaffix.hxx		\
	csutil.hxx		\
	dictmgr.hxx		\
	hashmgr.hxx		\
	htypes.hxx		\
	hunspell.h		\
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
	phonet.cxx		\
	phonet.hxx

if WITH_SYSTEM_MYSPELL
EXTRA_MYSPELL_FILES=
else
EXTRA_MYSPELL_FILES=$(COPIED_MYSPELL_FILES)
endif


INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(MYSPELL_CFLAGS) $(CXX_WARN_CFLAGS) -D_ENCHANT_BUILD=1

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

