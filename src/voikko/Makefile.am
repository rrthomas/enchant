if WITH_VOIKKO
target_lib = libenchant_voikko.la
else
target_lib =
endif

AM_CPPFLAGS = -I$(top_srcdir) $(ISYSTEM)$(top_builddir)/lib $(ISYSTEM)$(top_srcdir)/lib -I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(WARN_CFLAGS) $(VOIKKO_CFLAGS) -D_ENCHANT_BUILD=1

voikko_LTLIBRARIES = $(target_lib)
voikkodir= $(libdir)/enchant

libenchant_voikko_la_LIBADD= $(ENCHANT_LIBS) $(VOIKKO_LIBS) $(top_builddir)/src/libenchant.la
libenchant_voikko_la_LDFLAGS = -module -avoid-version -no-undefined
libenchant_voikko_la_SOURCES = voikko_provider.c
libenchant_voikko_lalibdir=$(libdir)/enchant
