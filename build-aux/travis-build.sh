#!/bin/bash
# Build on Travis
# Written by Reuben Thomas 2020.
# This file is in the public domain.

set -e

./bootstrap
CONFIGURE_ARGS=(--enable-relocatable --with-zemberek=check)
if [[ "$ASAN" == "yes" ]]; then
    CONFIGURE_ARGS+=(CFLAGS="-g3 -fsanitize=address -fsanitize=undefined" LDFLAGS="-fsanitize=address -fsanitize=undefined")
fi
./configure --enable-silent-rules "${CONFIGURE_ARGS[@]}"
make
make distcheck

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then sudo locale-gen fr_FR.UTF-8; env LANG=fr_FR.UTF-8 make check ; fi
