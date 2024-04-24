# Makefile.am for Enchant library
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

AM_VALAFLAGS = --debug --no-color --vapidir=. --pkg config --pkg internal --pkg posix --pkg gnu --pkg gio-2.0 --pkg gmodule-2.0
if OS_WIN32
AM_VALAFLAGS += -D OS_WIN32
endif
AM_CPPFLAGS = --include config.h -I$(top_srcdir) $(ISYSTEM)$(top_builddir)/libgnu $(ISYSTEM)$(top_srcdir)/libgnu $(GLIB_CFLAGS) $(WARN_CFLAGS) -DG_LOG_DOMAIN='"libenchant"'

# FIXME: Require Vala 0.58 when released, and remove copy of gnu.vapi
VAPIS =	config.vapi internal.vapi gnu.vapi

lib_LTLIBRARIES = libenchant-@ENCHANT_MAJOR_VERSION@.la

BUILT_SOURCES = dummy.vala

dummy.vala: $(VAPIS)
	touch $@

libenchant_@ENCHANT_MAJOR_VERSION@_la_CPPFLAGS = $(AM_CPPFLAGS) -D_ENCHANT_BUILD=1 -DENCHANT_VERSION_STRING=\"@ENCHANT_MAJOR_VERSION@.@ENCHANT_MINOR_VERSION@.@ENCHANT_MICRO_VERSION@\" -DENCHANT_MAJOR_VERSION=\"@ENCHANT_MAJOR_VERSION@\"
libenchant_@ENCHANT_MAJOR_VERSION@_la_LIBADD = $(top_builddir)/libgnu/libgnu.la $(GLIB_LIBS) $(LTLIBOBJS)

libenchant_@ENCHANT_MAJOR_VERSION@_la_LDFLAGS = -no-undefined -export-symbols-regex '^enchant_.*'
if OS_WIN32
libenchant_@ENCHANT_MAJOR_VERSION@_la_LDFLAGS += -avoid-version
else
libenchant_@ENCHANT_MAJOR_VERSION@_la_LDFLAGS += -version-info $(VERSION_INFO)
endif

libenchant_@ENCHANT_MAJOR_VERSION@_la_SOURCES = \
	enchant.h enchant-provider.h pwl.h \
	api.vala \
	broker.vala \
	dict.vala \
	provider.vala \
	pwl.vala \
	session.vala \
	util.vala \
	$(BUILT_SOURCES)
if OS_WIN32
libenchant_@ENCHANT_MAJOR_VERSION@_la_SOURCES += libenchant.rc
endif

libenchant_includedir = $(pkgincludedir)-@ENCHANT_MAJOR_VERSION@
libenchant_include_HEADERS = enchant.h enchant++.h

libenchant_datadir = $(pkgdatadir)-@ENCHANT_MAJOR_VERSION@
libenchant_data_DATA = enchant.ordering

dist_man_MANS = enchant.5
nodist_doc_DATA = enchant.html

edit = sed \
	-e 's|@PKGDATADIR[@]|$(pkgdatadir)|g' \
	-e 's|@SYSCONFDIR[@]|$(sysconfdir)|g'

DISTCLEANFILES = $(dist_man_MANS) $(nodist_doc_DATA) $(BUILT_SOURCES)

enchant.5: $(builddir)/enchant.5.in Makefile.am $(top_builddir)/config.status
	rm -f $@ $@.tmp
	$(edit) $(abs_builddir)/enchant.5.in >$@.tmp
	mv $@.tmp $@

enchant.html: enchant.5
	groff -mandoc -Thtml $< > $@

EXTRA_DIST = $(libenchant_data_DATA) $(VAPIS)

.rc.lo:
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --tag=RC --mode=compile $(RC) $(RCFLAGS) $< -o $@
