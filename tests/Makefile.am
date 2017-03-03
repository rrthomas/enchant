AM_CPPFLAGS = -I$(top_srcdir)/src $(ENCHANT_CFLAGS)

AM_TESTS_ENVIRONMENT = \
	ENCHANT_MODULE_PATH=$(top_builddir)/providers/@objdir@; \
	export ENCHANT_MODULE_PATH; \
	rm -f test.pwl; \
	cp $(srcdir)/test.pwl.orig $(builddir)/test.pwl; \
	chmod +w $(builddir)/test.pwl;

DISTCLEANFILES = test.pwl

EXTRA_DIST = test.pwl.orig

LDADD = $(top_builddir)/src/libenchant.la $(ENCHANT_LIBS)

check_PROGRAMS = enchant.test enchantxx.test

enchant_test_SOURCES = enchant.test.c
enchantxx_test_SOURCES = enchantxx.test.cpp

TESTS = $(check_PROGRAMS)

# Enforce serial running of tests, so they don't contend for test.pwl
enchantxx.log: enchant.log
