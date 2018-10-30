SUBDIRS = enchant_providers

# Get libdir suffix
if GNU_MAKE
libdir_subdir=$(shell echo "$(libdir)" | sed -e 's|^$(exec_prefix)/||' | sed -e 's|^/||')
else
libdir_subdir=lib
endif

AM_CPPFLAGS = -I$(top_srcdir)/src $(ENCHANT_CFLAGS) -DENCHANT_MAJOR_VERSION=\"@ENCHANT_MAJOR_VERSION@\"

ENCHANT_CONFIG_DIR = config

AM_TESTS_ENVIRONMENT = \
	export ENCHANT_CONFIG_DIR=$(ENCHANT_CONFIG_DIR); \
	if test -n "$(VALGRIND)"; then export VALGRIND='$(VALGRIND) --suppressions=$(top_srcdir)/lib/relocatable.valgrind'; fi; \
	export LIBTOOL=$(top_builddir)/libtool; \
	rm -rf test.pwl; \
	$(MAKE) libenchant-copy; \
	cp $(srcdir)/test.pwl.orig $(builddir)/test.pwl; \
	cp $(builddir)/@objdir@/*@shlibext@ .; \
	chmod +w $(builddir)/test.pwl;

DISTCLEANFILES = test.pwl *@shlibext@

distclean-local:
	rm -rf $(libdir_subdir) $(ENCHANT_CONFIG_DIR)

EXTRA_DIST = test.pwl.orig mock_provider.h run-test

LIBENCHANT_COPY = $(builddir)/$(libdir_subdir)/libenchant-@ENCHANT_MAJOR_VERSION@.la
$(LIBENCHANT_COPY): $(top_builddir)/src/libenchant-@ENCHANT_MAJOR_VERSION@.la
	$(MAKE) libenchant-copy

libenchant-copy:
	rm -rf $(libdir_subdir)
	$(MKDIR_P) $(libdir_subdir)/enchant-@ENCHANT_MAJOR_VERSION@
	cp -r $(top_builddir)/src/@objdir@ $(libdir_subdir)/
	cp $(top_builddir)/src/libenchant-@ENCHANT_MAJOR_VERSION@.la $(libdir_subdir)/

LDADD = $(LIBENCHANT_COPY) $(ENCHANT_LIBS)
LIBADD = $(LIBENCHANT_COPY)
DEPS = $(LIBENCHANT_COPY)

check_LTLIBRARIES = \
	libenchant_mock_provider.la \
	libenchant_mock_provider2.la \
	libenchant_null_provider.la \
	libenchant_null_identify.la \
	libenchant_null_describe.la

# Should not really use _ENCHANT_BUILD here, but seems to be necessary to get correct DLL exports for Windows
libenchant_mock_provider_la_CPPFLAGS=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) -D_ENCHANT_BUILD=1
# Adding -rpath to LDFLAGS causes the .so to be built even though the lib is not to be installed
libenchant_mock_provider_la_LDFLAGS = -module -avoid-version -no-undefined -rpath /foo $(ENCHANT_LIBS)
libenchant_mock_provider_la_SOURCES = mock_provider.cpp

# Build distinct libraries rather than copying a single one, to avoid being
# defeated by caching, so we can configure each one differently.
libenchant_mock_provider2_la_CPPFLAGS = $(libenchant_mock_provider_la_CPPFLAGS)
libenchant_mock_provider2_la_LDFLAGS = $(libenchant_mock_provider_la_LDFLAGS)
libenchant_mock_provider2_la_SOURCES = $(libenchant_mock_provider_la_SOURCES)

libenchant_null_provider_la_CPPFLAGS = $(libenchant_mock_provider_la_CPPFLAGS) -DENCHANT_TEST_NULL_PROVIDER
libenchant_null_provider_la_LDFLAGS = $(libenchant_mock_provider_la_LDFLAGS)
libenchant_null_provider_la_SOURCES = $(libenchant_mock_provider_la_SOURCES)

libenchant_null_identify_la_CPPFLAGS = $(libenchant_mock_provider_la_CPPFLAGS) -DENCHANT_TEST_NULL_IDENTIFY
libenchant_null_identify_la_LDFLAGS = $(libenchant_mock_provider_la_LDFLAGS)
libenchant_null_identify_la_SOURCES = $(libenchant_mock_provider_la_SOURCES)

libenchant_null_describe_la_CPPFLAGS = $(libenchant_mock_provider_la_CPPFLAGS) -DENCHANT_TEST_NULL_DESCRIBE
libenchant_null_describe_la_LDFLAGS = $(libenchant_mock_provider_la_LDFLAGS)
libenchant_null_describe_la_SOURCES = $(libenchant_mock_provider_la_SOURCES)

check_PROGRAMS = main.test
LOG_COMPILER = $(srcdir)/run-test

main_test_SOURCES = main.test.cpp \
	EnchantBrokerTestFixture.h \
	EnchantDictionaryTestFixture.h \
	EnchantTestFixture.h \
	mock_provider.h \
	dictionary/enchant_dict_add_tests.cpp \
	dictionary/enchant_dict_add_to_session_tests.cpp \
	dictionary/enchant_dict_check_tests.cpp \
	dictionary/enchant_dict_describe_tests.cpp \
	dictionary/enchant_dict_free_string_list_tests.cpp \
	dictionary/enchant_dict_get_error_tests.cpp \
	dictionary/enchant_dict_get_extra_word_characters_tests.cpp \
	dictionary/enchant_dict_is_added_tests.cpp \
	dictionary/enchant_dict_is_removed_tests.cpp \
	dictionary/enchant_dict_is_word_character_tests.cpp \
	dictionary/enchant_dict_remove_from_session_tests.cpp \
	dictionary/enchant_dict_remove_tests.cpp \
	dictionary/enchant_dict_store_replacement_tests.cpp \
	dictionary/enchant_dict_suggest_tests.cpp \
	broker/enchant_broker_describe_tests.cpp \
	broker/enchant_broker_dict_exists_tests.cpp \
	broker/enchant_broker_dict_exists_tests.i \
	broker/enchant_broker_free_dict_tests.cpp \
	broker/enchant_broker_free_tests.cpp \
	broker/enchant_broker_get_error_tests.cpp \
	broker/enchant_broker_init_tests.cpp \
	broker/enchant_broker_list_dicts_tests.cpp \
	broker/enchant_broker_request_dict_tests.cpp \
	broker/enchant_broker_request_pwl_dict_tests.cpp \
	broker/enchant_broker_set_ordering_tests.cpp \
	pwl/enchant_pwl_tests.cpp \
	provider/enchant_provider_broker_set_error_tests.cpp \
	provider/enchant_provider_dict_set_error_tests.cpp \
	provider/enchant_provider_get_prefix_dir_tests.cpp \
	provider/enchant_provider_get_user_config_dirs_tests.cpp \
	provider/enchant_provider_get_user_language_tests.cpp \
	$(NULL)
main_test_DEPENDENCIES = $(LIBENCHANT_COPY)
main_test_LDADD = $(LIBENCHANT_COPY) $(ENCHANT_LIBS) $(UNITTESTPP_LIBS)
main_test_CPPFLAGS = $(AM_CPPFLAGS) $(UNITTESTPP_CFLAGS) -DLIBDIR_SUBDIR=\"$(libdir_subdir)\"

TESTS = $(check_PROGRAMS)
