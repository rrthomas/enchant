SUBDIRS=. aspell uspell hunspell hspell applespell voikko zemberek

AM_CPPFLAGS = -I$(top_srcdir) $(ISYSTEM)$(top_builddir)/lib $(ISYSTEM)$(top_srcdir)/lib $(ENCHANT_CFLAGS) $(WARN_CFLAGS)

lib_LTLIBRARIES = libenchant.la

libenchant_la_CPPFLAGS = $(AM_CPPFLAGS) -DENCHANT_GLOBAL_MODULE_DIR=\"$(libdir)/enchant\" -DENCHANT_GLOBAL_ORDERING=\"$(datadir)/enchant\" -D_ENCHANT_BUILD=1 -DENCHANT_VERSION_STRING=\"@ENCHANT_MAJOR_VERSION@.@ENCHANT_MINOR_VERSION@.@ENCHANT_MICRO_VERSION@\"
libenchant_la_LIBADD= $(ENCHANT_LIBS) @ENCHANT_WIN32_RESOURCE@

libenchant_la_LDFLAGS = -no-undefined -export-symbols-regex '^enchant_.*'
if OS_WIN32
libenchant_la_LDFLAGS += -avoid-version
else
libenchant_la_LDFLAGS += -version-info $(VERSION_INFO)
endif

libenchant_la_DEPENDENCIES = @ENCHANT_WIN32_RESOURCE@
libenchant_la_SOURCES = enchant.c pwl.c enchant.h pwl.h

libenchant_includedir = $(includedir)/enchant
libenchant_include_HEADERS = enchant.h enchant-provider.h enchant++.h

if WITH_APPLESPELL
libenchant_la_CPPFLAGS += @APPLESPELL_CFLAGS@
libenchant_la_LIBADD += @APPLESPELL_LIBS@
libenchant_la_LDFLAGS += @APPLESPELL_LDFLAGS@
endif

LDADD = libenchant.la $(ENCHANT_LIBS)
bin_PROGRAMS = enchant-lsmod enchant
enchant_SOURCES = enchant-ispell.c

EXTRA_DIST=enchant.i

if OS_WIN32

@ENCHANT_WIN32_RESOURCE@ : libenchant.rc
	$(top_srcdir)/lt-compile-resource libenchant.rc @ENCHANT_WIN32_RESOURCE@
endif
