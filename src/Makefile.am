SUBDIRS=. aspell ispell uspell

INCLUDES=-I$(top_srcdir) $(ENCHANT_CFLAGS) -DENCHANT_GLOBAL_MODULE_DIR=\"$(libdir)/enchant\" -DENCHANT_GLOBAL_ORDERING=\"$(datadir)/enchant\"

lib_LTLIBRARIES = libenchant.la

libenchant_la_LIBADD= $(ENCHANT_LIBS)
libenchant_la_LDFLAGS = -version-info $(VERSION_INFO) -no-undefined
libenchant_la_SOURCES = enchant.c enchant.h

libenchant_includedir = $(includedir)/enchant/
libenchant_include_HEADERS = enchant.h enchant-provider.h enchant++.h
