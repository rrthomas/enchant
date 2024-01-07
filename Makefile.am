# Makefile.am for Enchant
#
# Copyright (C) 2003 Dom Lachowicz
#               2016-2024 Reuben Thomas
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.

ACLOCAL_AMFLAGS = -I m4
AM_DISTCHECK_CONFIGURE_FLAGS = --enable-relocatable

SUBDIRS = lib src providers tests

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
	COPYING.LIB	\
	Doxyfile	\
	doxygen/html	\
	m4/gnulib-cache.m4

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

.PHONY:	doxygen
doxygen:
	$(DOXYGEN) $(srcdir)/Doxyfile

dist: doxygen

loc:
	cloc --force-lang="Bourne Shell",conf $(ALL_SOURCE_FILES)

# Ignore built files that are part of the distribution (specifically,
# doxygen/*).
distcleancheck_listfiles = \
       find . -type f -exec sh -c 'test -f $(srcdir)/$$1 || echo $$1' \
	    sh '{}' ';'

release: distcheck
	git diff --exit-code && \
	git tag -a -m "Release tag" "v$(VERSION)" && \
	git push && git push --tags && \
	woger github \
		github_user=AbiWord \
		package=$(PACKAGE) \
		version=$(VERSION) \
		dist_type=tar.gz
