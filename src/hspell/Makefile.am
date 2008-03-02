if WITH_HSPELL
target_lib = libenchant_hspell.la
else
target_lib =
endif

AM_CPPFLAGS=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(CC_WARN_CFLAGS) $(HSPELL_CFLAGS) -D_ENCHANT_BUILD=1

hspell_LTLIBRARIES = $(target_lib)
hspelldir= $(libdir)/enchant

libenchant_hspell_la_LIBADD= $(ENCHANT_LIBS) $(HSPELL_LIBS) $(top_builddir)/src/libenchant.la
libenchant_hspell_la_LDFLAGS = -module -avoid-version -no-undefined
libenchant_hspell_la_SOURCES = hspell_provider.c
libenchant_hspell_lalibdir=$(libdir)/enchant
