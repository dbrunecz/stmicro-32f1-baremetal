#!/bin/bash

TC=/home/db/Downloads/gcc-arm-none-eabi-5_4-2016q3/bin/arm-none-eabi-
NM=${TC}nm
SZ=${TC}size

for V in "$@"; do
    echo "### ${V}"
    ${NM} --defined-only -td -S --size-sort ${V} | grep " t \| T "
    #${NM} --defined-only -td -S --size-sort ${V} | grep " d \| D \| r \| R "
    ${SZ} ${V}
done
