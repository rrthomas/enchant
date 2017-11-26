AM_CPPFLAGS = -I$(top_srcdir) $(ISYSTEM)$(top_builddir)/lib $(ISYSTEM)$(top_srcdir)/lib $(ENCHANT_CFLAGS) $(WARN_CFLAGS)
DEFS = -DENABLE_COSTLY_RELOCATABLE -DIN_LIBRARY -DINSTALLDIR=\"$(libdir)\"

lib_LTLIBRARIES = libenchant-@ENCHANT_MAJOR_VERSION@.la

libenchant_@ENCHANT_MAJOR_VERSION@_la_CPPFLAGS = $(AM_CPPFLAGS) -D_ENCHANT_BUILD=1 -DENCHANT_VERSION_STRING=\"@ENCHANT_MAJOR_VERSION@.@ENCHANT_MINOR_VERSION@.@ENCHANT_MICRO_VERSION@\" -DENCHANT_MAJOR_VERSION=\"@ENCHANT_MAJOR_VERSION@\"
libenchant_@ENCHANT_MAJOR_VERSION@_la_LIBADD = $(top_builddir)/lib/libgnu.la $(ENCHANT_LIBS) $(LTLIBOBJS)

libenchant_@ENCHANT_MAJOR_VERSION@_la_LDFLAGS = -no-undefined -export-symbols-regex '^enchant_.*'
if OS_WIN32
libenchant_@ENCHANT_MAJOR_VERSION@_la_LDFLAGS += -avoid-version
else
libenchant_@ENCHANT_MAJOR_VERSION@_la_LDFLAGS += -version-info $(VERSION_INFO)
endif

libenchant_@ENCHANT_MAJOR_VERSION@_la_SOURCES = lib.c pwl.c enchant.h pwl.h
if OS_WIN32
libenchant_@ENCHANT_MAJOR_VERSION@_la_SOURCES += libenchant.rc
endif

libenchant_includedir = $(pkgincludedir)-@ENCHANT_MAJOR_VERSION@
libenchant_include_HEADERS = enchant.h enchant-provider.h enchant++.h

pkgdata_DATA = enchant.ordering

dist_man_MANS = enchant.1 enchant-lsmod.1

edit = sed \
	-e 's|@datadir[@]|$(datadir)|g'

DISTCLEANFILES = enchant.1

enchant.1: $(srcdir)/enchant.1.in Makefile.am $(top_builddir)/config.status
	rm -f $@ $@.tmp
	$(edit) $(abs_srcdir)/enchant.1.in >$@.tmp
	mv $@.tmp $@

LDADD = libenchant-@ENCHANT_MAJOR_VERSION@.la $(ENCHANT_LIBS) $(top_builddir)/lib/libgnu.la
bin_PROGRAMS = enchant-@ENCHANT_MAJOR_VERSION@ enchant-lsmod-@ENCHANT_MAJOR_VERSION@
enchant_@ENCHANT_MAJOR_VERSION@_SOURCES = enchant.c
enchant_lsmod_@ENCHANT_MAJOR_VERSION@_SOURCES = enchant-lsmod.c

EXTRA_DIST = $(pkgdata_DATA) enchant.1.in

.rc.lo:
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --tag=RC --mode=compile $(RC) $(RCFLAGS) $< -o $@
