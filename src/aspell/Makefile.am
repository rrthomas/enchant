if OS_WIN32

install-libtool-import-lib:
	$(INSTALL) .libs/libenchant_aspell.dll.a $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libenchant_aspell.dll.a
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

# TODO: figure out how to install this in a subdir of $(libdir)

if WITH_PSPELL
target_lib = libenchant_aspell.la
else
target_lib =
endif

INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS)

lib_LTLIBRARIES = $(target_lib)

libenchant_aspell_la_LIBADD= $(ENCHANT_LIBS) $(PSPELL_LIBS)
libenchant_aspell_la_LDFLAGS = -version-info $(VERSION_INFO) -no-undefined
libenchant_aspell_la_SOURCES = aspell_provider.c

install-data-local: install-libtool-import-lib

uninstall-local: uninstall-libtool-import-lib
