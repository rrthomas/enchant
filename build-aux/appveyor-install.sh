#!/bin/sh
# Pre-install script for appveyor: install build deps

# Get mingw type, if any, from MSYSTEM
case $MSYSTEM in
    MINGW32)
        MINGW_ARCH=i686
        PREFIX=/mingw32
        ;;
    MINGW64)
        MINGW_ARCH=x86_64
        PREFIX=/mingw64
        ;;
esac

# GLib
pacman --noconfirm -S mingw-w64-$MINGW_ARCH-glib2 glib2-devel mingw-w64-$MINGW_ARCH-hunspell mingw-w64-$MINGW_ARCH-hunspell-en mingw-w64-$MINGW_ARCH-nuspell

# UnitTest++ is not packaged in mingw
wget https://github.com/unittest-cpp/unittest-cpp/releases/download/v1.6.1/unittest-cpp-1.6.1.tar.gz
tar zxvf unittest-cpp-1.6.1.tar.gz
cd unittest-cpp-1.6.1
./configure --prefix=$PREFIX && make && make install
cd ..
