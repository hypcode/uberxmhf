#!/bin/bash

set -e


if [[ $# -ne 2 ]]; then
    echo "usage: build <linux-kernel-src-dir> <cross-compile-tools-dir>"
        exit 1
fi

LINUX_KERNEL_SRC=$1
CROSS_COMPILE_TOOLS_DIR=$2

echo "Building module..."

make -C $LINUX_KERNEL_SRC ARCH=arm \
CROSS_COMPILE=$CROSS_COMPILE_TOOLS_DIR/arm-linux-gnueabihf- \
M=$PWD

echo "Module build success!"

