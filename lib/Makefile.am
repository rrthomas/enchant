## Makefile for the lib subdirectory of Enchant

# Initialize variables that will be added to by Makefile.gnulib
EXTRA_DIST =
BUILT_SOURCES =
MOSTLYCLEANFILES =
MOSTLYCLEANDIRS =
CLEANFILES =
noinst_LTLIBRARIES =

# Set compilation flags needed for relocatable.c used by relocatable-lib-lgpl
AM_CPPFLAGS = -DIN_LIBRARY -DENABLE_COSTLY_RELOCATABLE

if SHLIBS_IN_BINDIR
AM_CPPFLAGS += -DINSTALLDIR=\"$(bindir)\"
else
AM_CPPFLAGS += -DINSTALLDIR=\"$(libdir)\"
endif

# Include auto-generated makefile
include Makefile.gnulib
