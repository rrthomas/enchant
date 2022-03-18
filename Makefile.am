ACLOCAL_AMFLAGS = -I m4
AM_DISTCHECK_CONFIGURE_FLAGS = --enable-relocatable

SUBDIRS = src providers tests

# Note that the template file is called library.pc.in, but generates a
# versioned .pc file using some magic in AC_CONFIG_FILES.
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = enchant-$(ENCHANT_MAJOR_VERSION).pc

DISTCLEANFILES = $(pkgconfig_DATA)
EXTRA_DIST =		\
	enchant.pc.in	\
	README 		\
	HACKING 	\
	NEWS 		\
	ChangeLog 	\
	AUTHORS 	\
	COPYING.LIB

ALL_SOURCE_FILES = \
	bootstrap.conf \
	configure.ac \
	Makefile.am \
	src/Makefile.am \
	src/*.[chi] \
	providers/Makefile.am \
	providers/*.[ch] \
	providers/*.cpp \
	providers/*.mm

loc:
	cloc --force-lang="Bourne Shell",conf $(ALL_SOURCE_FILES)

release: distcheck
	git diff --exit-code && \
	git tag -a -m "Release tag" "v$(VERSION)" && \
	git push && git push --tags && \
	woger github \
		github_user=AbiWord \
		package=$(PACKAGE) \
		version=$(VERSION) \
		dist_type=tar.gz
