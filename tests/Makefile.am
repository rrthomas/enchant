AM_CPPFLAGS = -I$(top_srcdir)/src $(ENCHANT_CFLAGS)

# Note: give library directory with and without $(ENCHANT_LT_OBJDIR) to
# work with static builds and on systems that do not have static compilation
# and hence don't use LT_OBJDIR.
# FIXME: Following line is not portable to MingW (need semicolons instead of colons)
AM_TESTS_ENVIRONMENT = \
	ENCHANT_MODULE_PATH=$(top_builddir)/providers/$(ENCHANT_LT_OBJDIR):$(top_builddir)/src/providers; \
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
