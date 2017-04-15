EXTRA_DIST =			\
	AppleSpell.config

provider_LTLIBRARIES =
providerdir = $(pkglibdir)

AM_CPPFLAGS = -I$(top_srcdir) $(ISYSTEM)$(top_builddir)/lib $(ISYSTEM)$(top_srcdir)/lib -I$(top_srcdir)/src $(ENCHANT_CFLAGS)  -D_ENCHANT_BUILD=1
AM_CFLAGS = $(WARN_CFLAGS)
AM_CXXFLAGS = $(WARN_CXXFLAGS)
AM_LDFLAGS = -module -avoid-version -no-undefined $(ENCHANT_LIBS) $(top_builddir)/src/libenchant.la $(top_builddir)/lib/libgnu.la

if WITH_ASPELL
provider_LTLIBRARIES += enchant_aspell.la
endif

if WITH_HSPELL
provider_LTLIBRARIES += enchant_hspell.la
endif

if WITH_HUNSPELL
provider_LTLIBRARIES += enchant_hunspell.la
endif
enchant_hunspell_la_CXXFLAGS = $(AM_CXXFLAGS) $(HUNSPELL_CFLAGS)
enchant_hunspell_la_LIBADD = $(HUNSPELL_LIBS)
enchant_hunspell_la_SOURCES = enchant_hunspell.cpp

if WITH_VOIKKO
provider_LTLIBRARIES += enchant_voikko.la
endif
enchant_voikko_la_CXXFLAGS = $(AM_CXXFLAGS) $(VOIKKO_CFLAGS)
enchant_voikko_la_LIBADD = $(VOIKKO_LIBS)

if WITH_ZEMBEREK
provider_LTLIBRARIES += enchant_zemberek.la
endif
enchant_zemberek_la_CXXFLAGS = $(AM_CXXFLAGS) $(ZEMBEREK_CFLAGS)
enchant_zemberek_la_LIBADD = $(ZEMBEREK_LIBS)
enchant_zemberek_la_SOURCES = enchant_zemberek.cpp

if WITH_APPLESPELL
provider_LTLIBRARIES += enchant_applespell.la
endif
enchant_applespell_la_LIBTOOLFLAGS = $(AM_LIBTOOLFLAGS) --tag=CXX
enchant_applespell_la_OBJCXXFLAGS = $(AM_OBJCXXFLAGS)
enchant_applespell_la_SOURCES = applespell_checker.mm
