AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)/lib -I$(top_srcdir)/lib $(GLIB_CFLAGS)
DEFS = -DNO_XMALLOC -DENABLE_COSTLY_RELOCATABLE -DIN_LIBRARY -DINSTALLDIR=\"$(libdir)\"

lib_LTLIBRARIES = libenchant.la

libenchant_la_CPPFLAGS = $(AM_CPPFLAGS) -D_ENCHANT_BUILD=1
libenchant_la_LIBADD = $(top_builddir)/lib/libgnu.la $(GLIB_LIBS) $(LTLIBOBJS)

libenchant_la_SOURCES = lib.c enchant.h
