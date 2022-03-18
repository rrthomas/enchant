AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)/lib -I$(top_srcdir)/lib $(ENCHANT_CFLAGS)

configmake.h: Makefile
	$(AM_V_GEN)rm -f $@-t && \
	{ echo '/* DO NOT EDIT! GENERATED AUTOMATICALLY! */'; \
	  echo '#define SYSCONFDIR "$(sysconfdir)"'; \
	  echo '#define PKGDATADIR "$(pkgdatadir)"'; \
	  echo '#define PKGLIBDIR "$(pkglibdir)"'; \
	} | sed '/""/d' > $@-t && \
	mv -f $@-t $@

BUILT_SOURCES = configmake.h
CLEANFILES = configmake.h configmake.h-t

lib_LTLIBRARIES = libenchant-@ENCHANT_MAJOR_VERSION@.la

# Set compilation flags needed for relocatable.c used by relocatable-lib-lgpl
relocatable_cppflags = -DIN_LIBRARY -DENABLE_COSTLY_RELOCATABLE \
	-DNO_XMALLOC=1 #-DENABLE_RELOCATABLE and -DINSTALLPREFIX defined in config.h
if SHLIBS_IN_BINDIR
relocatable_cppflags += -DINSTALLDIR=\"$(bindir)\"
else
relocatable_cppflags += -DINSTALLDIR=\"$(libdir)\"
endif

libenchant_@ENCHANT_MAJOR_VERSION@_la_CPPFLAGS = $(AM_CPPFLAGS) -D_ENCHANT_BUILD=1 \
	-DENCHANT_VERSION_STRING=\"@ENCHANT_MAJOR_VERSION@.@ENCHANT_MINOR_VERSION@.@ENCHANT_MICRO_VERSION@\" \
	-DENCHANT_MAJOR_VERSION=\"@ENCHANT_MAJOR_VERSION@\" $(relocatable_cppflags)

libenchant_@ENCHANT_MAJOR_VERSION@_la_LIBADD = $(ENCHANT_LIBS)

libenchant_@ENCHANT_MAJOR_VERSION@_la_LDFLAGS = -no-undefined -export-symbols-regex '^enchant_.*'
if OS_WIN32
libenchant_@ENCHANT_MAJOR_VERSION@_la_LDFLAGS += -avoid-version
else
libenchant_@ENCHANT_MAJOR_VERSION@_la_LDFLAGS += -version-info $(VERSION_INFO)
endif

libenchant_@ENCHANT_MAJOR_VERSION@_la_SOURCES = lib.c pwl.c enchant.h pwl.h relocatable.c relocatable.h
if OS_WIN32
libenchant_@ENCHANT_MAJOR_VERSION@_la_SOURCES += libenchant.rc
endif

libenchant_includedir = $(pkgincludedir)-@ENCHANT_MAJOR_VERSION@
libenchant_include_HEADERS = enchant.h enchant-provider.h enchant++.h
noinst_HEADERS = debug.h

pkgdata_DATA = enchant.ordering

dist_man_MANS = enchant-@ENCHANT_MAJOR_VERSION@.1 enchant-lsmod-@ENCHANT_MAJOR_VERSION@.1

edit = sed \
	-e 's|DATADIR|$(datadir)|g'

DISTCLEANFILES = $(dist_man_MANS)

enchant-@ENCHANT_MAJOR_VERSION@.1: $(builddir)/enchant.1 Makefile.am $(top_builddir)/config.status
	rm -f $@ $@.tmp
	$(edit) $(abs_builddir)/enchant.1 >$@.tmp
	mv $@.tmp $@

enchant-lsmod-@ENCHANT_MAJOR_VERSION@.1: $(builddir)/enchant-lsmod.1 Makefile.am $(top_builddir)/config.status
	cp $(abs_builddir)/enchant-lsmod.1 $@

LDADD = libenchant-@ENCHANT_MAJOR_VERSION@.la $(ENCHANT_LIBS)
bin_PROGRAMS = enchant-@ENCHANT_MAJOR_VERSION@ enchant-lsmod-@ENCHANT_MAJOR_VERSION@
enchant_@ENCHANT_MAJOR_VERSION@_SOURCES = enchant.c
enchant_lsmod_@ENCHANT_MAJOR_VERSION@_SOURCES = enchant-lsmod.c

EXTRA_DIST = $(pkgdata_DATA) enchant.1.in enchant-lsmod.1.in

.rc.lo:
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --tag=RC --mode=compile $(RC) $(RCFLAGS) $< -o $@
