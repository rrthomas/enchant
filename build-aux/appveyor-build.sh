#!/bin/bash
# Build on AppVeyor
# Written by Reuben Thomas 2020.
# This file is in the public domain.

set -e

./bootstrap
CONFIGURE_ARGS=(--enable-relocatable --with-zemberek=check)
if [[ "$ASAN" == "yes" ]]; then
    CONFIGURE_ARGS+=(CFLAGS="-g3 -fsanitize=address -fsanitize=undefined" CXXFLAGS="-g3 -fsanitize=address -fsanitize=undefined" LDFLAGS="-fsanitize=address -fsanitize=undefined")
fi
./configure "${CONFIGURE_ARGS[@]}"
make
make distcheck

if [[ "$APPVEYOR_BUILD_WORKER_IMAGE" == "Ubuntu" ]]; then sudo locale-gen fr_FR.UTF-8; env LANG=fr_FR.UTF-8 make check ; fi
