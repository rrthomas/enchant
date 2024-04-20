# Makefile.am for Enchant commands
#
# Copyright (C) 2003 Dom Lachowicz
#               2016-2024 Reuben Thomas
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.

AM_CPPFLAGS = -I$(top_srcdir) $(ISYSTEM)$(top_builddir)/libgnu $(ISYSTEM)$(top_srcdir)/libgnu -I$(top_srcdir)/lib $(GLIB_CFLAGS) $(WARN_CFLAGS) -DG_LOG_DOMAIN='"libenchant"'

dist_man_MANS = enchant-@ENCHANT_MAJOR_VERSION@.1 enchant-lsmod-@ENCHANT_MAJOR_VERSION@.1
nodist_doc_DATA = enchant-@ENCHANT_MAJOR_VERSION@.html enchant-lsmod-@ENCHANT_MAJOR_VERSION@.html

edit = sed \
	-e 's|@PKGDATADIR[@]|$(pkgdatadir)|g' \
	-e 's|@SYSCONFDIR[@]|$(sysconfdir)|g'

DISTCLEANFILES = $(dist_man_MANS) $(nodist_doc_DATA)

.1.html:
	groff -mandoc -Thtml $< > $@

enchant-@ENCHANT_MAJOR_VERSION@.1: $(builddir)/enchant.1 Makefile.am $(top_builddir)/config.status
	cp $(abs_builddir)/enchant.1 $@

enchant-lsmod-@ENCHANT_MAJOR_VERSION@.1: $(builddir)/enchant-lsmod.1 Makefile.am $(top_builddir)/config.status
	cp $(abs_builddir)/enchant-lsmod.1 $@

LDADD = $(top_builddir)/lib/libenchant-@ENCHANT_MAJOR_VERSION@.la $(GLIB_LIBS) $(top_builddir)/libgnu/libgnu.la
bin_PROGRAMS = enchant-@ENCHANT_MAJOR_VERSION@ enchant-lsmod-@ENCHANT_MAJOR_VERSION@
enchant_@ENCHANT_MAJOR_VERSION@_SOURCES = enchant.c
enchant_lsmod_@ENCHANT_MAJOR_VERSION@_SOURCES = enchant-lsmod.c

EXTRA_DIST = enchant.1.in enchant-lsmod.1.in
