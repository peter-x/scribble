#!/bin/sh

export PATH="/opt/onyx/arm/bin:/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/:$PATH"
export ONYX_SDK_ROOT=/opt/onyx/arm
export PKG_CONFIG_PATH=/opt/onyx/arm/lib/pkgconfig/
export QMAKESPEC=/opt/onyx/arm/mkspecs/qws/linux-arm-g++/

arm_cc="/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-linux-gcc"
arm_cxx="/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-linux-g++"
compiler_prefix=ccache
compiler_prefix=""

mkdir -p build/arm
cd build/arm
CC="$compiler_prefix$arm_cc" CXX="$compiler_prefix$arm_cxx" cmake -DBUILD_FOR_ARM:BOOL=ON ../..
make
