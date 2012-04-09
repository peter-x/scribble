#!/bin/sh

export ONYX_SDK_ROOT=/opt/onyx/arm
export QMAKESPEC=/opt/onyx/arm/mkspecs/qws/linux-x86-g++/

mkdir -p build/x86
cd build/x86
# XXX detect if we need dbus in the .pro file
qmake ../../scribble.pro LIBS+=-lonyx_touch QT+=dbus
make
