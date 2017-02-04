AM_CPPFLAGS = -I$(top_srcdir)/src $(ENCHANT_CFLAGS)

# Note: each directory is given with and without $(ENCHANT_LT_OBJDIR) to
# work with static builds and on systems that do not have static compilation
# and hence don't use LT_OBJDIR.
# FIXME: Following line is ugly (but inserting backslashes stops it
# working), and not portable to Windows (need semicolons instead of colons)
AM_TESTS_ENVIRONMENT = \
	ENCHANT_MODULE_PATH=$(top_builddir)/src/applespell/$(ENCHANT_LT_OBJDIR):$(top_builddir)/src/applespell:$(top_builddir)/src/aspell/$(ENCHANT_LT_OBJDIR):$(top_builddir)/src/aspell:$(top_builddir)/src/hspell/$(ENCHANT_LT_OBJDIR):$(top_builddir)/src/hspell:$(top_builddir)/src/ispell/$(ENCHANT_LT_OBJDIR):$(top_builddir)/src/ispell:$(top_builddir)/src/myspell/$(ENCHANT_LT_OBJDIR):$(top_builddir)/src/myspell:$(top_builddir)/src/uspell/$(ENCHANT_LT_OBJDIR):$(top_builddir)/src/uspell:$(top_builddir)/src/voikko/$(ENCHANT_LT_OBJDIR):$(top_builddir)/src/voikko:$(top_builddir)/src/zemberek/$(ENCHANT_LT_OBJDIR):$(top_builddir)/src/zemberek; \
	export ENCHANT_MODULE_PATH; \
	rm -f test.pwl; \
	cp $(srcdir)/test.pwl.orig $(builddir)/test.pwl; \
	chmod +w $(builddir)/test.pwl;

DISTCLEANFILES = test.pwl

EXTRA_DIST = test.pwl.orig ispell.in

LDADD = $(top_builddir)/src/libenchant.la $(ENCHANT_LIBS)

check_PROGRAMS = enchant.test enchantxx.test
bin_PROGRAMS = enchant-lsmod enchant

enchant_test_SOURCES = enchant.test.c
enchant_SOURCES = enchant-ispell.c
enchantxx_test_SOURCES = enchantxx.test.cpp
enchant_lsmod_SOURCES = enchant-lsmod.c

TESTS = $(check_PROGRAMS)

# Enforce serial running of tests, so they don't contend for test.pwl
enchantxx.log: enchant.log
