AM_CPPFLAGS = -I$(top_srcdir)/src $(ENCHANT_CFLAGS)

AM_TESTS_ENVIRONMENT = \
	ENCHANT_PREFIX_DIR=""; \
	export ENCHANT_PREFIX_DIR; \
	rm -f test.pwl enchant; \
	mkdir -p lib/enchant; \
	cp $(top_builddir)/providers/@objdir@/*@shlibext@ lib/enchant || :;
	cp $(srcdir)/test.pwl.orig $(builddir)/test.pwl; \
	chmod +w $(builddir)/test.pwl;

DISTCLEANFILES = test.pwl lib/enchant/*@shlibext@

EXTRA_DIST = test.pwl.orig

LDADD = $(top_builddir)/src/libenchant.la $(ENCHANT_LIBS)

check_PROGRAMS = enchant.test enchantxx.test

enchant_test_SOURCES = enchant.test.c
enchantxx_test_SOURCES = enchantxx.test.cpp

TESTS = $(check_PROGRAMS)

# Enforce serial running of tests, so they don't contend for test.pwl
enchantxx.log: enchant.log
