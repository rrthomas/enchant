if OS_WIN32

install-libtool-import-lib:
	$(INSTALL) .libs/libenchant_aspell.dll.a $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libenchant_aspell.dll.a
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

if WITH_PSPELL
target_lib = libenchant_aspell.la
else
target_lib =
endif

INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS)

aspell_LTLIBRARIES = $(target_lib)
aspelldir= $(libdir)/enchant

libenchant_aspell_la_LIBADD= $(ENCHANT_LIBS) $(PSPELL_LIBS)
libenchant_aspell_la_LDFLAGS = -version-info $(VERSION_INFO) -no-undefined
libenchant_aspell_la_SOURCES = aspell_provider.c
libenchant_aspell_lalibdir=$(libdir)/enchant

install-data-local: install-libtool-import-lib

uninstall-local: uninstall-libtool-import-lib
