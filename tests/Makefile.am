INCLUDES=-I$(top_srcdir)/src

DEPS= $(top_builddir)/src/libenchant.la
ldadd= $(top_builddir)/src/libenchant.la  $(ENCHANT_LIBS)

if WITH_CXX
cxx_progs = test-enchantxx
else
cxx_progs =
endif

noinst_PROGRAMS = test-enchant $(cxx_progs)

test_enchant_SOURCES = test-enchant.c
test_enchant_LDFLAGS =
test_enchant_DEPENDENCIES = $(DEPS)
test_enchant_LDADD = $(ldadd)

test_enchantxx_SOURCES = test-enchantxx.cpp
test_enchantxx_LDFLAGS =
test_enchantxx_DEPENDENCIES = $(DEPS)
test_enchantxx_LDADD = $(ldadd)

