SUBDIRS = enchant_providers mock_provider

AM_CPPFLAGS = -I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(UNITTESTPP_CFLAGS) -DLIBDIR_SUBDIR=\"$(libdir_subdir)\"

ENCHANT_CONFIG_DIR="config"

AM_TESTS_ENVIRONMENT = \
	export ENCHANT_CONFIG_DIR=$(ENCHANT_CONFIG_DIR); \
	cp $(builddir)/mock_provider/@objdir@/*@shlibext@ .;

check_PROGRAMS = main
TESTS = main

LIBENCHANT_COPY = $(builddir)/$(libdir_subdir)/libenchant.la
$(LIBENCHANT_COPY): $(top_builddir)/src/libenchant.la
	rm -rf $(libdir_subdir)/
	$(MKDIR_P) $(libdir_subdir)/enchant
	cp -r $(top_builddir)/src/@objdir@ $(libdir_subdir)/
	cp $(top_builddir)/src/libenchant.la $(libdir_subdir)/

main_SOURCES = main.cpp \
	EnchantBrokerTestFixture.h \
	EnchantDictionaryTestFixture.h \
	EnchantTestFixture.h \
	mock_provider/mock_provider.h \
	dictionary/enchant_dict_add_tests.cpp \
	dictionary/enchant_dict_add_to_session_tests.cpp \
	dictionary/enchant_dict_check_tests.cpp \
	dictionary/enchant_dict_describe_tests.cpp \
	dictionary/enchant_dict_free_string_list_tests.cpp \
	dictionary/enchant_dict_get_error_tests.cpp \
	dictionary/enchant_dict_is_added_tests.cpp \
	dictionary/enchant_dict_is_removed_tests.cpp \
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
main_DEPENDENCIES = $(LIBENCHANT_COPY)
main_LDADD = $(LIBENCHANT_COPY) $(ENCHANT_LIBS) $(UNITTESTPP_LIBS)

DISTCLEANFILES = *@shlibext@ $(libdir_subdir)/enchant/*@shlibext@ config/

distclean-local:
	rm -rf $(libdir_subdir) $(ENCHANT_CONFIG_DIR)
