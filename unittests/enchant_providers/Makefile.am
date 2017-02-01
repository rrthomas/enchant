AM_CPPFLAGS = -I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(UNITTESTPP_CFLAGS)

# FIXME: .so extension will not work on Windows
AM_TESTS_ENVIRONMENT = \
	cp $(builddir)/../mock_provider/*.so $(builddir)/../mock_provider/.libs/*.so . || :; \
	cp $(top_builddir)/src/*/*.so $(top_builddir)/src/*/.libs/*.so . || :;

DEPS = $(top_builddir)/src/libenchant.la
ldadd = $(top_builddir)/src/libenchant.la $(ENCHANT_LIBS)

if WITH_CXX
cxx_progs = main
else
cxx_progs =
endif

check_PROGRAMS = $(cxx_progs)
TESTS = main

main_SOURCES = main.cpp \
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
main_LDFLAGS =
main_DEPENDENCIES = $(DEPS)
main_LDADD = $(ldadd) $(UNITTESTPP_LIBS)

DISTCLEANFILES = libenchant_*.so
