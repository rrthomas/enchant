if OS_WIN32

install-libtool-import-lib:
	$(INSTALL) .libs/libenchant_uspell.dll.a $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libenchant_uspell.dll.a
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

if WITH_USPELL
target_lib = libenchant_uspell.la
else
target_lib =
endif

INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(USPELL_CFLAGS)

uspell_LTLIBRARIES = $(target_lib)
uspelldir= $(libdir)/enchant

libenchant_uspell_la_LIBADD= $(ENCHANT_LIBS) $(USPELL_LIBS) $(top_builddir)/src/libenchant.la
libenchant_uspell_la_LDFLAGS = -version-info $(VERSION_INFO) -no-undefined
libenchant_uspell_la_SOURCES = uspell_provider.cpp
libenchant_uspell_lalibdir=$(libdir)/enchant

install-data-local: install-libtool-import-lib

uninstall-local: uninstall-libtool-import-lib
