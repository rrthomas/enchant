if WITH_ZEMBEREK
target_lib = libenchant_zemberek.la
else
target_lib =
endif

INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(ZEMBEREK_CFLAGS) $(CXX_WARN_CFLAGS) -D_ENCHANT_BUILD=1

zemberek_LTLIBRARIES = $(target_lib)
zemberekdir= $(libdir)/enchant

libenchant_zemberek_lalibdir=$(libdir)/enchant
libenchant_zemberek_la_LIBADD= $(ZEMBEREK_LIBS) $(top_builddir)/src/libenchant.la
libenchant_zemberek_la_LDFLAGS = -avoid-version -no-undefined

libenchant_zemberek_la_SOURCES =\
	zemberek.cpp		\
	zemberek.h		\
	zemberek_provider.cpp


