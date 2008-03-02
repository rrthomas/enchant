if WITH_VOIKKO
target_lib = libenchant_voikko.la
else
target_lib =
endif

INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(CC_WARN_CFLAGS) @VOIKKO_INC@ -D_ENCHANT_BUILD=1

voikko_LTLIBRARIES = $(target_lib)
voikkodir= $(libdir)/enchant

libenchant_voikko_la_LIBADD= $(ENCHANT_LIBS) $(VOIKKO_LIBS) $(top_builddir)/src/libenchant.la
libenchant_voikko_la_LDFLAGS = -module -avoid-version -no-undefined
libenchant_voikko_la_SOURCES = voikko_provider.c
libenchant_voikko_lalibdir=$(libdir)/enchant
