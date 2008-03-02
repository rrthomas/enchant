if WITH_USPELL
target_lib = libenchant_uspell.la
else
target_lib =
endif

INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(USPELL_CFLAGS) $(CXX_WARN_CFLAGS) -D_ENCHANT_BUILD=1

uspell_LTLIBRARIES = $(target_lib)
uspelldir= $(libdir)/enchant

libenchant_uspell_la_LIBADD= $(ENCHANT_LIBS) $(USPELL_LIBS) $(top_builddir)/src/libenchant.la
libenchant_uspell_la_LDFLAGS = -module -avoid-version -no-undefined
libenchant_uspell_la_SOURCES = uspell_provider.cpp
libenchant_uspell_lalibdir=$(libdir)/enchant
