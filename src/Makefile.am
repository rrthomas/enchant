# Makefile.am for Enchant commands
#
# Copyright (C) 2003 Dom Lachowicz
#               2016-2025 Reuben Thomas
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.

AM_VALAFLAGS = --debug --no-color --vapidir=. --vapidir=$(top_srcdir)/lib --pkg config --pkg configmake --pkg posix --pkg gio-2.0 --pkg gmodule-2.0 --pkg enchant-@ENCHANT_MAJOR_VERSION@
AM_CPPFLAGS = --include config.h -I$(top_srcdir) $(ISYSTEM)$(top_builddir)/libgnu $(ISYSTEM)$(top_srcdir)/libgnu -I$(top_srcdir)/lib $(GLIB_CFLAGS) $(WARN_VALA_CFLAGS) -DENCHANT_MAJOR_VERSION=\"@ENCHANT_MAJOR_VERSION@\"

VAPIS = config.vapi enchant-@ENCHANT_MAJOR_VERSION@.vapi

dummy.vala: $(BUILT_VAPIS) $(VAPIS)
	touch $@

BUILT_SOURCES = dummy.vala $(BUILT_VAPIS) util.h

BUILT_VAPIS = util.vapi

util.h util.vapi: libutil.la

dist_man_MANS = enchant-@ENCHANT_MAJOR_VERSION@.1 enchant-lsmod-@ENCHANT_MAJOR_VERSION@.1
nodist_doc_DATA = enchant-@ENCHANT_MAJOR_VERSION@.html enchant-lsmod-@ENCHANT_MAJOR_VERSION@.html

DISTCLEANFILES = $(dist_man_MANS) $(nodist_doc_DATA) dummy.vala

.1.html:
	groff -mandoc -Thtml $< > $@

enchant-@ENCHANT_MAJOR_VERSION@.1: $(builddir)/enchant.1 Makefile.am $(top_builddir)/config.status
	cp $(abs_builddir)/enchant.1 $@

enchant-lsmod-@ENCHANT_MAJOR_VERSION@.1: $(builddir)/enchant-lsmod.1 Makefile.am $(top_builddir)/config.status
	cp $(abs_builddir)/enchant-lsmod.1 $@

noinst_LTLIBRARIES = libutil.la
libutil_la_SOURCES = util.vala
libutil_la_VALAFLAGS = $(AM_VALAFLAGS) --vapi=util.vapi --header=util.h
libutil_la_LIBADD = $(GLIB_LIBS)

LDADD = $(top_builddir)/lib/libenchant-@ENCHANT_MAJOR_VERSION@.la $(GLIB_LIBS) $(top_builddir)/libgnu/libgnu.la libutil.la
bin_PROGRAMS = enchant-@ENCHANT_MAJOR_VERSION@ enchant-lsmod-@ENCHANT_MAJOR_VERSION@
enchant_@ENCHANT_MAJOR_VERSION@_SOURCES = enchant.vala
enchant_@ENCHANT_MAJOR_VERSION@_VALAFLAGS = $(AM_VALAFLAGS) --pkg util
enchant_lsmod_@ENCHANT_MAJOR_VERSION@_SOURCES = enchant-lsmod.vala
enchant_lsmod_@ENCHANT_MAJOR_VERSION@_VALAFLAGS = $(AM_VALAFLAGS) --pkg util

EXTRA_DIST = enchant.1.in enchant-lsmod.1.in util.h $(VAPIS)

loc-local:
	$(CLOC) $(ALL_SOURCE_FILES)

ALL_SOURCE_FILES = \
	Makefile.am \
	*.vala
