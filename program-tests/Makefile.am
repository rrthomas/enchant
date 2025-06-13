# Tests Makefile.am
#
# Copyright (c) 2021-2024 Reuben Thomas <rrt@sc3d.org>
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.

TEST_EXTENSIONS = .sh
SH_LOG_COMPILER = $(srcdir)/run-test

if OS_WIN32
DIFF="diff --strip-trailing-cr"
PATH_CONVERT="cygpath --mixed"
else
DIFF=diff
PATH_CONVERT=echo
endif

EMPTY =

TESTS = \
	misspelled-input.sh \
	unknown-option.sh \
	zero-arguments.sh \
	$(EMPTY)

RESULTS = \
	misspelled-input-expected.txt \
	unknown-option-expected.txt \
	zero-arguments-expected.txt \
	$(EMPTY)

# Run tests serially, as "make install" commands cannot run in parallel.
unknown-option.log: misspelled-input.log
zero-arguments.log: unknown-option.log

EXTRA_DIST = \
	$(SH_LOG_COMPILER) \
	$(TESTS) \
	$(INPUTS) \
	$(RESULTS) \
	misspelled-input.txt \
	en.aff \
	en.dic \
	$(EMPTY)

AM_TESTS_ENVIRONMENT = \
	export MAKE=$(MAKE); \
	export EXEEXT=$(EXEEXT); \
	export LC_MESSAGES=C; \
	export prefix=$(prefix); \
	export sysconfdir=$(sysconfdir); \
	export bindir=$(bindir); \
	export libdir=$(libdir); \
	export abs_srcdir=$(abs_srcdir); \
	export DIFF=$(DIFF); \
	export PATH_CONVERT=$(PATH_CONVERT); \
	export ENCHANT_MAJOR_VERSION=$(ENCHANT_MAJOR_VERSION); \
	export LSAN_OPTIONS=suppressions=$(abs_srcdir)/asan-suppressions.txt:fast_unwind_on_malloc=0:print_suppressions=0;
