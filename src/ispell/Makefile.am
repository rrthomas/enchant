if WITH_ISPELL
target_lib = libenchant_ispell.la
else
target_lib =
endif

INCLUDES=-I$(top_srcdir)/src $(ENCHANT_CFLAGS) $(ISPELL_CFLAGS) -D_ENCHANT_BUILD=1

ispell_LTLIBRARIES = $(target_lib)
ispelldir= $(libdir)/enchant

libenchant_ispell_lalibdir=$(libdir)/enchant
libenchant_ispell_la_LIBADD= $(ENCHANT_LIBS) $(top_builddir)/src/libenchant.la
libenchant_ispell_la_LDFLAGS = -version-info $(VERSION_INFO) -no-undefined
libenchant_ispell_la_SOURCES =	\
	correct.cpp		\
	good.cpp		\
	hash.cpp		\
	lookup.cpp		\
	makedent.cpp		\
	tgood.cpp		\
	ispell_checker.h	\
	ispell_def.h		\
	ispell.h		\
	msgs.h			\
	sp_spell.h		\
	ispell_checker.cpp
