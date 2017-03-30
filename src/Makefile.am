AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)/lib -I$(top_srcdir)/lib $(ENCHANT_CFLAGS)
DEFS = -DNO_XMALLOC -DENABLE_COSTLY_RELOCATABLE -DIN_LIBRARY -DINSTALLDIR=\"$(libdir)\"

lib_LTLIBRARIES = libenchant.la

libenchant_la_CPPFLAGS = $(AM_CPPFLAGS) -D_ENCHANT_BUILD=1
libenchant_la_LIBADD = $(top_builddir)/lib/libgnu.la $(ENCHANT_LIBS) $(LTLIBOBJS)

libenchant_la_SOURCES = lib.c pwl.c enchant.h pwl.h

libenchant_includedir = $(includedir)/enchant
libenchant_include_HEADERS = enchant.h enchant-provider.h
