## Makefile.am for the providers subdirectory of Enchant
#
# Copyright (C) 2017-2024 Reuben Thomas
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.

EXTRA_DIST = AppleSpell.config

provider_LTLIBRARIES =
providerdir = $(pkglibdir)-@ENCHANT_MAJOR_VERSION@
libenchant_datadir = $(pkgdatadir)-@ENCHANT_MAJOR_VERSION@

AM_CPPFLAGS = -I$(top_srcdir) $(ISYSTEM)$(top_builddir)/libgnu $(ISYSTEM)$(top_srcdir)/libgnu -I$(top_srcdir)/lib $(GLIB_CFLAGS) -D_ENCHANT_BUILD=1
AM_CFLAGS = $(WARN_CFLAGS)
AM_CXXFLAGS = $(WARN_CXXFLAGS)
AM_LDFLAGS = -module -avoid-version -no-undefined $(GLIB_LIBS) $(top_builddir)/lib/libenchant-@ENCHANT_MAJOR_VERSION@.la $(top_builddir)/libgnu/libgnu.la

if WITH_ASPELL
provider_LTLIBRARIES += enchant_aspell.la
endif

if WITH_HSPELL
provider_LTLIBRARIES += enchant_hspell.la
endif

if WITH_HUNSPELL
provider_LTLIBRARIES += enchant_hunspell.la
endif
enchant_hunspell_la_CXXFLAGS = $(AM_CXXFLAGS) $(HUNSPELL_CFLAGS)
enchant_hunspell_la_LIBADD = $(HUNSPELL_LIBS)
enchant_hunspell_la_SOURCES = enchant_hunspell.cpp

if WITH_NUSPELL
provider_LTLIBRARIES += enchant_nuspell.la
endif
enchant_nuspell_la_CXXFLAGS = $(AM_CXXFLAGS) $(NUSPELL_CFLAGS) -std=c++17
enchant_nuspell_la_LIBADD = $(NUSPELL_LIBS)
enchant_nuspell_la_SOURCES = enchant_nuspell.cpp

if WITH_VOIKKO
provider_LTLIBRARIES += enchant_voikko.la
endif
enchant_voikko_la_CFLAGS = $(AM_CFLAGS) $(VOIKKO_CFLAGS)
enchant_voikko_la_LIBADD = $(VOIKKO_LIBS)

if WITH_WIN8
provider_LTLIBRARIES += enchant_win8.la
endif
enchant_win8_la_CXXFLAGS = $(AM_CXXFLAGS) $(WIN8_CFLAGS)
enchant_win8_la_LIBADD = $(WIN8_LIBS)
enchant_win8_la_SOURCES = enchant_win8.cpp

if WITH_ZEMBEREK
provider_LTLIBRARIES += enchant_zemberek.la
endif
enchant_zemberek_la_CXXFLAGS = $(AM_CXXFLAGS) $(ZEMBEREK_CFLAGS)
enchant_zemberek_la_LIBADD = $(ZEMBEREK_LIBS)
enchant_zemberek_la_SOURCES = enchant_zemberek.cpp

if WITH_APPLESPELL
provider_LTLIBRARIES += enchant_applespell.la
libenchant_data_DATA = AppleSpell.config
endif
enchant_applespell_la_LIBTOOLFLAGS = $(AM_LIBTOOLFLAGS) --tag=CXX
enchant_applespell_la_OBJCXXFLAGS = $(AM_OBJCXXFLAGS)
enchant_applespell_la_SOURCES = applespell_checker.mm

install-data-hook:
	cd $(DESTDIR)$(providerdir) && $(RM) $(provider_LTLIBRARIES) *.a

# Removing files in install-data-hook seems to confuse libtool's uninstall mode,
# so it fails to remove the .so files. Remove them for it.
uninstall-hook:
	cd $(DESTDIR)$(providerdir) && $(RM) $(provider_LTLIBRARIES) *.a *@shlibext@

loc:
	$(CLOC) $(ALL_SOURCE_FILES)

ALL_SOURCE_FILES = \
	Makefile.am \
	*.[ch] \
	*.cpp \
	*.mm
