if WITH_ZEMBEREK
target_lib = libenchant_zemberek.la
else
target_lib =
endif

INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) -D_ENCHANT_BUILD=1 @ZEMBEREK_CFLAGS@

zemberek_LTLIBRARIES = $(target_lib)
zemberekdir= $(libdir)/enchant

libenchant_zemberek_lalibdir=$(libdir)/enchant
libenchant_zemberek_la_LIBADD= $(top_builddir)/src/libenchant.la $(SOCKET_LIBS)
libenchant_zemberek_la_LDFLAGS = -avoid-version -no-undefined

libenchant_zemberek_la_SOURCES = zemberek_provider.cpp


