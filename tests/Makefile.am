INCLUDES=-I$(top_srcdir)/src

DEPS= $(top_builddir)/src/libenchant.la
ldadd= $(top_builddir)/src/libenchant.la  $(ENCHANT_LIBS)

noinst_PROGRAMS = test-enchant

test_enchant_SOURCES = test-enchant.c
test_enchant_LDFLAGS =
test_enchant_DEPENDENCIES = $(DEPS)
test_enchant_LDADD = $(ldadd)
