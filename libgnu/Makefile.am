## Makefile.am for the libgnu subdirectory of Enchant
#
# Copyright (C) 2017-2024 Reuben Thomas
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.

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
AM_CPPFLAGS += -DINSTALLDIR=BINDIR -include configmake.h
else
AM_CPPFLAGS += -DINSTALLDIR=LIBDIR -include configmake.h
endif

loc:
	$(CLOC) $(ALL_SOURCE_FILES)

ALL_SOURCE_FILES = libgnu/Makefile.am

# Include auto-generated makefile
include Makefile.gnulib
