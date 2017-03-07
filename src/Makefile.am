AM_CPPFLAGS = -I$(top_srcdir) $(ISYSTEM)$(top_builddir)/lib $(ISYSTEM)$(top_srcdir)/lib $(ENCHANT_CFLAGS) $(WARN_CFLAGS)
DEFS = -DENABLE_COSTLY_RELOCATABLE -DIN_LIBRARY -DINSTALLDIR=\"$(libdir)\"

lib_LTLIBRARIES = libenchant.la

libenchant_la_CPPFLAGS = $(AM_CPPFLAGS) -D_ENCHANT_BUILD=1 -DENCHANT_VERSION_STRING=\"@ENCHANT_MAJOR_VERSION@.@ENCHANT_MINOR_VERSION@.@ENCHANT_MICRO_VERSION@\"
libenchant_la_LIBADD = $(top_builddir)/lib/libgnu.la $(ENCHANT_LIBS) $(LTLIBOBJS)

libenchant_la_LDFLAGS = -no-undefined -export-symbols-regex '^enchant_.*'
if OS_WIN32
libenchant_la_LDFLAGS += -avoid-version
else
libenchant_la_LDFLAGS += -version-info $(VERSION_INFO)
endif

libenchant_la_SOURCES = lib.c pwl.c enchant.h pwl.h
if OS_WIN32
libenchant_la_SOURCES += libenchant.rc
endif

libenchant_includedir = $(includedir)/enchant
libenchant_include_HEADERS = enchant.h enchant-provider.h enchant++.h

orderingdir=$(datadir)/enchant
ordering_DATA = enchant.ordering

man_MANS = enchant.1

LDADD = libenchant.la $(ENCHANT_LIBS)
bin_PROGRAMS = enchant-lsmod enchant

EXTRA_DIST = enchant.i $(ordering_DATA) $(man_MANS)

.rc.lo:
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --tag=RC --mode=compile $(RC) $(RCFLAGS) $< -o $@
