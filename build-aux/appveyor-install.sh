#!/bin/sh
# Pre-install script for appveyor

pacman --noconfirm -S glib2-devel
wget http://repo.msys2.org/mingw/x86_64/mingw-w64-x86_64-hunspell-en-2016.11.20-2-any.pkg.tar.xz
pacman --noconfirm -U mingw-w64-x86_64-hunspell-en-2016.11.20-2-any.pkg.tar.xz
wget https://github.com/hunspell/hunspell/archive/v1.6.0.tar.gz
tar zxvf v1.6.0.tar.gz || true # Error in unpacking (symlink README before file README.md)
cd hunspell-1.6.0 && ln -s README.md README && autoreconf -vfi && ./configure --prefix=/usr && make && make install
wget https://github.com/unittest-cpp/unittest-cpp/releases/download/v1.6.1/unittest-cpp-1.6.1.tar.gz
tar zxvf unittest-cpp-1.6.1.tar.gz
cd unittest-cpp-1.6.1 && ./configure --prefix=/usr && make && make install
