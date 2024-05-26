# libenchant -- Generic spell checking library

Maintainer: Reuben Thomas  
Home page: https://abiword.github.io/enchant/  
Bug reports and other issues: https://github.com/AbiWord/enchant  

libenchant is licensed under the terms of the GNU LGPL (see the file
COPYING.LIB), with a special exception allowing the use of proprietary
spell-checking systems.

Enchant aims to provide a simple but comprehensive abstraction for dealing
with different spell checking libraries in a consistent way. A client, such
as a text editor or word processor, need not know anything about a specific
spell-checker, and since all back-ends are plugins, new spell-checkers can
be added without needing any change to the program using Enchant.

Enchant can be configured by the user, who can even add spell-checker
plugins if desired.

Enchant works with the following spell-checkers:

* Hunspell (formerly Myspell)
* Nuspell
* GNU Aspell
* Hspell
* Voikko
* Apple Spell (macOS only)
* Zemberek

Enchant is written in Vala, C and C++, and its only external dependency is
Glib. C and C++ compilers are required to build it.

Enchant bindings are supplied for C and C++. API documentation is available
in the doxygen/html directory, or online (see the home page). There are
third-party bindings for various languages (see the home page).


## Installation

Users should install from a release tarball. See the file INSTALL for
instructions.

Developers or others wishing to install from a git repository, see HACKING.


## Usage

See the man pages for information on how to use Enchant. In particular,
information about how to configure which spelling checker to use for each
language and how to work with personal word lists is in the man page
enchant(5). To see this man page, in a terminal, give the command:

    man 5 enchant
