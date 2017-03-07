AM_CPPFLAGS = -I$(top_srcdir)/src $(ENCHANT_CFLAGS)

ENCHANT_CONFIG_DIR = config

AM_TESTS_ENVIRONMENT = \
	export ENCHANT_CONFIG_DIR=$(ENCHANT_CONFIG_DIR); \
	rm -rf test.pwl $(libdir_subdir); \
	mkdir -p $(libdir_subdir)/enchant; \
	cp -r $(top_builddir)/src/@objdir@ $(libdir_subdir)/; \
	cp $(top_builddir)/src/libenchant.la $(libdir_subdir)/; \
	cp $(top_builddir)/providers/@objdir@/*@shlibext@ $(libdir_subdir)/enchant || :; \
	cp $(srcdir)/test.pwl.orig $(builddir)/test.pwl; \
	chmod +w $(builddir)/test.pwl;

DISTCLEANFILES = test.pwl

distclean-local:
	rm -rf $(libdir_subdir) $(ENCHANT_CONFIG_DIR)

EXTRA_DIST = test.pwl.orig

# FIXME: Remove this duplicate stanza when build system is made non-recursive
LIBENCHANT_COPY = $(builddir)/$(libdir_subdir)/libenchant.la
$(LIBENCHANT_COPY): $(top_builddir)/src/libenchant.la
	rm -rf $(libdir_subdir)/
	$(MKDIR_P) $(libdir_subdir)/enchant
	cp -r $(top_builddir)/src/@objdir@ $(libdir_subdir)/
	cp $(top_builddir)/src/libenchant.la $(libdir_subdir)/

LDADD = $(LIBENCHANT_COPY) $(ENCHANT_LIBS)
DEPS = $(LIBENCHANT_COPY)

check_PROGRAMS = enchant.test enchantxx.test

enchant_test_SOURCES = enchant.test.c
enchantxx_test_SOURCES = enchantxx.test.cpp

TESTS = $(check_PROGRAMS)

# Enforce serial running of tests, so they don't contend for test.pwl
enchantxx.log: enchant.log
