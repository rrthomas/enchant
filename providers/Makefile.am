# FIXME: Build and test AppleSpell back-end
EXTRA_DIST =			\
	applespell_checker.h	\
	applespell_checker.mm	\
	AppleSpell.config	\
	libenchant_hunspell.rc

provider_LTLIBRARIES =
providerdir = $(pkglibdir)

# FIXME: WARN_C/CXXFLAGS should be set once, per language
AM_CPPFLAGS = -I$(top_srcdir) $(ISYSTEM)$(top_builddir)/lib $(ISYSTEM)$(top_srcdir)/lib -I$(top_srcdir)/src $(ENCHANT_CFLAGS)  -D_ENCHANT_BUILD=1
AM_LDFLAGS = -module -avoid-version -no-undefined

if WITH_ASPELL
provider_LTLIBRARIES += libenchant_aspell.la
endif
libenchant_aspell_la_CPPFLAGS = $(AM_CPPFLAGS) $(WARN_CFLAGS)
libenchant_aspell_la_LIBADD = $(ENCHANT_LIBS) $(ASPELL_LIBS) $(top_builddir)/src/libenchant.la
libenchant_aspell_la_SOURCES = aspell_provider.c

if WITH_HSPELL
provider_LTLIBRARIES += libenchant_hspell.la
endif
libenchant_hspell_la_CPPFLAGS = $(AM_CPPFLAGS) $(WARN_CFLAGS)
libenchant_hspell_la_LIBADD = $(ENCHANT_LIBS) $(HSPELL_LIBS) $(top_builddir)/src/libenchant.la
libenchant_hspell_la_SOURCES = hspell_provider.c

if WITH_HUNSPELL
provider_LTLIBRARIES += libenchant_hunspell.la
endif
libenchant_hunspell_la_CPPFLAGS = $(AM_CPPFLAGS) $(WARN_CXXFLAGS) $(HUNSPELL_CFLAGS)
libenchant_hunspell_la_LIBADD = $(ENCHANT_LIBS) $(HUNSPELL_LIBS) $(top_builddir)/src/libenchant.la
libenchant_hunspell_la_SOURCES = hunspell_checker.cpp

if WITH_USPELL
provider_LTLIBRARIES += libenchant_uspell.la
endif
libenchant_uspell_la_CPPFLAGS = $(AM_CPPFLAGS) $(WARN_CXXFLAGS) $(USPELL_CFLAGS)
libenchant_uspell_la_LIBADD = $(ENCHANT_LIBS) $(USPELL_LIBS) $(top_builddir)/src/libenchant.la
libenchant_uspell_la_SOURCES = uspell_provider.cpp

if WITH_VOIKKO
provider_LTLIBRARIES += libenchant_voikko.la
endif
libenchant_voikko_la_CPPFLAGS = $(AM_CPPFLAGS) $(WARN_CFLAGS) $(VOIKKO_CFLAGS)
libenchant_voikko_la_LIBADD = $(ENCHANT_LIBS) $(VOIKKO_LIBS) $(top_builddir)/src/libenchant.la
libenchant_voikko_la_SOURCES = voikko_provider.c

if WITH_ZEMBEREK
provider_LTLIBRARIES += libenchant_zemberek.la
endif
libenchant_zemberek_la_CPPFLAGS = $(AM_CPPFLAGS) $(WARN_CXXFLAGS) $(ZEMBEREK_CFLAGS)
libenchant_zemberek_la_LIBADD = $(ENCHANT_LIBS) $(ZEMBEREK_LIBS) $(top_builddir)/src/libenchant.la
libenchant_zemberek_la_SOURCES =\
	zemberek.cpp		\
	zemberek.h		\
	zemberek_provider.cpp
