INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(CXX_WARN_CFLAGS)

EXTRA_DIST=test.pwl ispell.in

DEPS= $(top_builddir)/src/libenchant.la
ldadd= $(top_builddir)/src/libenchant.la  $(ENCHANT_LIBS)

if WITH_CXX
cxx_progs = test-enchantxx
else
cxx_progs =
endif

noinst_PROGRAMS = test-enchant $(cxx_progs)
bin_PROGRAMS= enchant-lsmod enchant

test_enchant_SOURCES = test-enchant.c
test_enchant_LDFLAGS =
test_enchant_DEPENDENCIES = $(DEPS)
test_enchant_LDADD = $(ldadd)

enchant_SOURCES = enchant-ispell.c
enchant_LDFLAGS =
enchant_DEPENDENCIES = $(DEPS)
enchant_LDADD = $(ldadd)

test_enchantxx_SOURCES = test-enchantxx.cpp
test_enchantxx_LDFLAGS =
test_enchantxx_DEPENDENCIES = $(DEPS)
test_enchantxx_LDADD = $(ldadd)

enchant_lsmod_SOURCES = enchant-lsmod.c
enchant_lsmod_LDFLAGS =
enchant_lsmod_DEPENDENCIES = $(DEPS)
enchant_lsmod_LDADD = $(ldadd)
