## Makefile.am for the tests subdirectory of Enchant
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

# Get libdir suffix
if GNU_MAKE
libdir_subdir=$(shell echo "$(libdir)" | sed -e 's|^$(exec_prefix)/||' | sed -e 's|^/||')
else
libdir_subdir=lib
endif

AM_CPPFLAGS = -I$(top_srcdir)/lib $(GLIB_CFLAGS) -DENCHANT_MAJOR_VERSION=\"@ENCHANT_MAJOR_VERSION@\"

ENCHANT_CONFIG_DIR = config

AM_TESTS_ENVIRONMENT = \
	export ENCHANT_CONFIG_DIR=$(ENCHANT_CONFIG_DIR); \
	export LIBTOOL=$(top_builddir)/libtool; \
	rm -f test.pwl; \
	$(MAKE) libenchant-copy; \
	cp $(srcdir)/test.pwl.orig $(builddir)/test.pwl; \
	cp $(builddir)/@objdir@/*@shlibext@ .; \
	cp $(top_builddir)/providers/@objdir@/*@shlibext@ $(builddir) || :; \
	chmod +w $(builddir)/test.pwl; \
	export LSAN_OPTIONS=suppressions=$(srcdir)/asan-suppressions.txt:fast_unwind_on_malloc=0;

DISTCLEANFILES = test.pwl *@shlibext@

distclean-local:
	rm -rf $(libdir_subdir) $(ENCHANT_CONFIG_DIR)

EXTRA_DIST = test.pwl.orig mock_provider.h run-test

LIBENCHANT_COPY = $(builddir)/$(libdir_subdir)/libenchant-@ENCHANT_MAJOR_VERSION@.la
$(LIBENCHANT_COPY): $(top_builddir)/lib/libenchant-@ENCHANT_MAJOR_VERSION@.la
	$(MAKE) libenchant-copy

libenchant-copy:
	rm -rf $(libdir_subdir)
	$(MKDIR_P) $(libdir_subdir)/enchant-@ENCHANT_MAJOR_VERSION@
	cp -r $(top_builddir)/lib/@objdir@ $(libdir_subdir)/
	cp $(top_builddir)/lib/libenchant-@ENCHANT_MAJOR_VERSION@.la $(libdir_subdir)/

LDADD = $(LIBENCHANT_COPY) $(GLIB_LIBS)
DEPS = $(LIBENCHANT_COPY)

check_LTLIBRARIES = \
	libenchant_mock_provider.la \
	libenchant_mock_provider2.la \
	libenchant_null_provider.la \
	libenchant_null_identify.la \
	libenchant_null_describe.la

# Should not really use _ENCHANT_BUILD here, but seems to be necessary to get correct DLL exports for Windows
libenchant_mock_provider_la_CPPFLAGS=-I$(top_srcdir)/lib $(GLIB_CFLAGS) -D_ENCHANT_BUILD=1
# Adding -rpath to LDFLAGS causes the .so to be built even though the lib is not to be installed
libenchant_mock_provider_la_LDFLAGS = -module -avoid-version -no-undefined -rpath /foo
libenchant_mock_provider_la_LIBADD = $(LIBENCHANT_COPY) $(GLIB_LIBS)
libenchant_mock_provider_la_SOURCES = mock_provider.cpp

# Build distinct libraries rather than copying a single one, to avoid being
# defeated by caching, so we can configure each one differently.
libenchant_mock_provider2_la_CPPFLAGS = $(libenchant_mock_provider_la_CPPFLAGS)
libenchant_mock_provider2_la_LDFLAGS = $(libenchant_mock_provider_la_LDFLAGS)
libenchant_mock_provider2_la_LIBADD = $(LIBENCHANT_COPY)
libenchant_mock_provider2_la_SOURCES = $(libenchant_mock_provider_la_SOURCES)

libenchant_null_provider_la_CPPFLAGS = $(libenchant_mock_provider_la_CPPFLAGS) -DENCHANT_TEST_NULL_PROVIDER
libenchant_null_provider_la_LDFLAGS = $(libenchant_mock_provider_la_LDFLAGS)
libenchant_null_provider_la_LIBADD = $(LIBENCHANT_COPY)
libenchant_null_provider_la_SOURCES = $(libenchant_mock_provider_la_SOURCES)

libenchant_null_identify_la_CPPFLAGS = $(libenchant_mock_provider_la_CPPFLAGS) -DENCHANT_TEST_NULL_IDENTIFY
libenchant_null_identify_la_LDFLAGS = $(libenchant_mock_provider_la_LDFLAGS)
libenchant_null_identify_la_LIBADD = $(LIBENCHANT_COPY)
libenchant_null_identify_la_SOURCES = $(libenchant_mock_provider_la_SOURCES)

libenchant_null_describe_la_CPPFLAGS = $(libenchant_mock_provider_la_CPPFLAGS) -DENCHANT_TEST_NULL_DESCRIBE
libenchant_null_describe_la_LDFLAGS = $(libenchant_mock_provider_la_LDFLAGS)
libenchant_null_describe_la_LIBADD = $(LIBENCHANT_COPY)
libenchant_null_describe_la_SOURCES = $(libenchant_mock_provider_la_SOURCES)

check_PROGRAMS = main.test providers.test
LOG_COMPILER = $(srcdir)/run-test
# Make the tests run serially.
providers.log: main.log

main_test_SOURCES = main.test.cpp \
	EnchantBrokerTestFixture.h \
	EnchantDictionaryTestFixture.h \
	EnchantTestFixture.h \
	mock_provider.h \
	dictionary/add.cpp \
	dictionary/add.i \
	dictionary/add_to_session.cpp \
	dictionary/add_to_session.i \
	dictionary/check.cpp \
	dictionary/check.i \
	dictionary/describe.cpp \
	dictionary/describe.i \
	dictionary/free_string_list.cpp \
	dictionary/get_error.cpp \
	dictionary/get_extra_word_characters.cpp \
	dictionary/is_added.cpp \
	dictionary/is_removed.cpp \
	dictionary/is_word_character.cpp \
	dictionary/remove_from_session.cpp \
	dictionary/remove_from_session.i \
	dictionary/remove.cpp \
	dictionary/suggest.cpp \
	dictionary/suggest.i \
	broker/describe.cpp \
	broker/dict_exists.cpp \
	broker/dict_exists.i \
	broker/free_dict.cpp \
	broker/free.cpp \
	broker/get_error.cpp \
	broker/init.cpp \
	broker/list_dicts.cpp \
	broker/request_dict.cpp \
	broker/request_dict_with_pwl.cpp \
	broker/request_pwl_dict.cpp \
	broker/set_ordering.cpp \
	pwl/pwl.cpp \
	pwl/pwl.i \
	provider/broker_set_error.cpp \
	provider/dict_set_error.cpp \
	provider/dict_set_error.i \
	provider/get_prefix_dir.cpp \
	provider/get_user_config_dirs.cpp \
	$(NULL)
main_test_DEPENDENCIES = $(LIBENCHANT_COPY)
main_test_LDADD = $(LIBENCHANT_COPY) $(GLIB_LIBS) $(UNITTESTPP_LIBS)
main_test_CPPFLAGS = $(AM_CPPFLAGS) $(UNITTESTPP_CFLAGS) -DLIBDIR_SUBDIR=\"$(libdir_subdir)\"

APPLESPELL_CONFIG = config/AppleSpell.config
$(APPLESPELL_CONFIG): $(top_srcdir)/providers/AppleSpell.config
	$(MKDIR_P) config
	cp $(top_srcdir)/providers/AppleSpell.config config/

providers_test_SOURCES = providers/providers.test.cpp \
	EnchantTestFixture.h \
	EnchantBrokerTestFixture.h \
	EnchantDictionaryTestFixture.h \
	providers/unittest_enchant_providers.h \
	providers/dictionary/check.cpp \
	providers/dictionary/suggest.cpp \
	providers/provider/describe_dict.cpp \
	providers/provider/dictionary_exists.cpp \
	providers/provider/identify_dict.cpp \
	providers/provider/list_dicts.cpp \
	providers/provider/request_dict.cpp \
	$(NULL)
providers_test_DEPENDENCIES = $(LIBENCHANT_COPY) $(APPLESPELL_CONFIG)
providers_test_LDADD = $(LIBENCHANT_COPY) $(GLIB_LIBS) $(UNITTESTPP_LIBS)
providers_test_CPPFLAGS = $(AM_CPPFLAGS) $(UNITTESTPP_CFLAGS) -DLIBDIR_SUBDIR=\"$(libdir_subdir)\"

TESTS = $(check_PROGRAMS)
