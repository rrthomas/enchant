SUBDIRS=aspell ispell uspell

if OS_WIN32

install-libtool-import-lib:
	$(INSTALL) .libs/libenchant.dll.a $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libenchant.dll.a
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

INCLUDES=-I$(top_srcdir) $(ENCHANT_CFLAGS) -DENCHANT_GLOBAL_MODULE_DIR=\"$(libdir)/enchant\"

lib_LTLIBRARIES = libenchant.la

libenchant_la_LIBADD= $(ENCHANT_LIBS)
libenchant_la_LDFLAGS = -version-info $(VERSION_INFO) -no-undefined
libenchant_la_SOURCES = enchant.c enchant.h

libenchant_includedir = $(includedir)/enchant/
libenchant_include_HEADERS = enchant.h enchant++.h

install-data-local: install-libtool-import-lib

uninstall-local: uninstall-libtool-import-lib
