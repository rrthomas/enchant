EXTRA_DIST = mock_provider.h

# Should not really use _ENCHANT_BUILD here, but seems to be necessary to get correct DLL exports for Windows
AM_CPPFLAGS=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) -D_ENCHANT_BUILD=1

# Need to build this for installation to get .so file for testing
check_LTLIBRARIES = \
	libenchant_mock_provider.la \
	libenchant_mock_provider2.la \
	libenchant_null_provider.la \
	libenchant_null_identify.la \
	libenchant_null_describe.la

# Adding -rpath to LDFLAGS causes the .so to be built even though the lib is noinst_
AM_LDFLAGS = -module -avoid-version -no-undefined -rpath /foo $(ENCHANT_LIBS) $(top_builddir)/src/libenchant.la
libenchant_mock_provider_la_SOURCES = mock_provider.cpp

# Build distinct libraries rather than copying a single one, to avoid being
# defeated by caching, so we can configure each one differently.
libenchant_mock_provider2_la_SOURCES = $(libenchant_mock_provider_la_SOURCES)

libenchant_null_provider_la_CPPFLAGS = $(AM_CPPFLAGS) -DENCHANT_TEST_NULL_PROVIDER
libenchant_null_provider_la_SOURCES = $(libenchant_mock_provider_la_SOURCES)

libenchant_null_identify_la_CPPFLAGS = $(AM_CPPFLAGS) -DENCHANT_TEST_NULL_IDENTIFY
libenchant_null_identify_la_SOURCES = $(libenchant_mock_provider_la_SOURCES)

libenchant_null_describe_la_CPPFLAGS = $(AM_CPPFLAGS) -DENCHANT_TEST_NULL_DESCRIBE
libenchant_null_describe_la_SOURCES = $(libenchant_mock_provider_la_SOURCES)
