if WITH_ISPELL
target_lib = libenchant_ispell.la
else
target_lib =
endif

INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(CXX_WARN_CFLAGS) $(ISPELL_CFLAGS) -D_ENCHANT_BUILD=1

ispell_LTLIBRARIES = $(target_lib)
ispelldir= $(libdir)/enchant

libenchant_ispell_lalibdir=$(libdir)/enchant
libenchant_ispell_la_LIBADD= $(ENCHANT_LIBS) $(top_builddir)/src/libenchant.la
libenchant_ispell_la_LDFLAGS = -module -avoid-version -no-undefined
libenchant_ispell_la_SOURCES =	\
	correct.cpp		\
	good.cpp		\
	hash.cpp		\
	ispell_checker.cpp	\
	ispell_checker.h	\
	ispell_def.h		\
	ispell.h		\
	lookup.cpp		\
	makedent.cpp		\
	msgs.h			\
	sp_spell.h		\
	tgood.cpp
