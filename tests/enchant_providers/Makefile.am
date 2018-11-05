# FIXME: common up with tests/Makefile.am

# Get libdir suffix
if GNU_MAKE
libdir_subdir=$(shell echo "$(libdir)" | sed -e 's|^$(exec_prefix)/||' | sed -e 's|^/||')
else
libdir_subdir=lib
endif

AM_CPPFLAGS = -I$(top_srcdir)/src $(ENCHANT_CFLAGS)

ENCHANT_CONFIG_DIR = config

AM_TESTS_ENVIRONMENT = \
	export ENCHANT_CONFIG_DIR=$(ENCHANT_CONFIG_DIR); \
	if test -n "$(VALGRIND)"; then export VALGRIND='$(VALGRIND) --suppressions=$(top_srcdir)/lib/relocatable.valgrind'; fi; \
	export LIBTOOL=$(top_builddir)/libtool; \
	$(MAKE) libenchant-copy; \
	cp $(top_builddir)/providers/@objdir@/*@shlibext@ $(builddir) || :;

DISTCLEANFILES = *@shlibext@

distclean-local:
	rm -rf $(libdir_subdir) $(ENCHANT_CONFIG_DIR)

LIBENCHANT_COPY = $(builddir)/$(libdir_subdir)/libenchant-@ENCHANT_MAJOR_VERSION@.la
$(LIBENCHANT_COPY): $(top_builddir)/src/libenchant-@ENCHANT_MAJOR_VERSION@.la
	$(MAKE) libenchant-copy

libenchant-copy:
	rm -rf $(libdir_subdir)
	$(MKDIR_P) $(libdir_subdir)/enchant-@ENCHANT_MAJOR_VERSION@
	cp -r $(top_builddir)/src/@objdir@ $(libdir_subdir)/
	cp $(top_builddir)/src/libenchant-@ENCHANT_MAJOR_VERSION@.la $(libdir_subdir)/

APPLESPELL_CONFIG = config/AppleSpell.config
$(APPLESPELL_CONFIG): $(top_srcdir)/providers/AppleSpell.config
	$(MKDIR_P) config
	cp $(top_srcdir)/providers/AppleSpell.config config/

LDADD = $(LIBENCHANT_COPY) $(ENCHANT_LIBS)
LIBADD = $(LIBENCHANT_COPY)
DEPS = $(LIBENCHANT_COPY)

check_PROGRAMS = providers.test
LOG_COMPILER = $(srcdir)/../run-test

providers_test_SOURCES = providers.test.cpp \
	../EnchantTestFixture.h \
	../EnchantBrokerTestFixture.h \
	../EnchantDictionaryTestFixture.h \
	unittest_enchant_providers.h \
	Dictionary/dictionary_check.cpp \
	Dictionary/dictionary_suggest.cpp \
	Provider/provider_describe_dict.cpp \
	Provider/provider_dictionary_exists.cpp \
	Provider/provider_identify_dict.cpp \
	Provider/provider_list_dicts.cpp \
	Provider/provider_request_dict.cpp \
	$(NULL)
providers_test_DEPENDENCIES = $(LIBENCHANT_COPY) $(APPLESPELL_CONFIG)
providers_test_LDADD = $(LIBENCHANT_COPY) $(ENCHANT_LIBS) $(UNITTESTPP_LIBS)
providers_test_CPPFLAGS = $(AM_CPPFLAGS) $(UNITTESTPP_CFLAGS) -DLIBDIR_SUBDIR=\"$(libdir_subdir)\"

TESTS = $(check_PROGRAMS)
