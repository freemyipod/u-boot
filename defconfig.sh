#!/bin/bash

set -eu

MAKE=gmake
BOARDS=("$@")

if [[ ${#BOARDS[@]} -eq 0 ]]; then
    BOARDS=(n36 n46 n58s n33 n20 n31 n25)
fi

cd ~/Git/u-boot

for BOARD in "${BOARDS[@]}"
do
  echo "$BOARD"
  DEFCONFIG="apple_${BOARD}_defconfig"
  "$MAKE" distclean
  rm -f generated_defconfig-e
  "$MAKE" "$DEFCONFIG"
  "$MAKE" savedefconfig
  mv defconfig "configs/${DEFCONFIG}"
done
