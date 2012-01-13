
# Make sure you already have cmake 2.6 installed. For cross compiling.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Additional cmake modules
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

Project(Scribble CXX C)
MESSAGE(Compiler: ${CMAKE_C_COMPILER})

# Check to use arm toolchain or not
IF(BUILD_FOR_ARM)
    INCLUDE(arm_toolchain)
    USE_ARM_TOOLCHAIN()
ENDIF(BUILD_FOR_ARM)

IF(UNIX OR BUILD_FOR_ARM)
    ADD_DEFINITIONS(-DSHARE_ROOT="/usr/share")
ELSE (UNIX OR BUILD_FOR_ARM)
    ADD_DEFINITIONS(-DSHARE_ROOT="")
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF (UNIX OR BUILD_FOR_ARM)

ADD_DEFINITIONS(-DCONFIG_CTRL_IFACE)
IF (WIN32)
ADD_DEFINITIONS(-DCONFIG_NATIVE_WINDOWS)
ADD_DEFINITIONS(-DCONFIG_CTRL_IFACE_NAMED_PIPE)
ELSE (WIN32)
ADD_DEFINITIONS(-DCONFIG_CTRL_IFACE_UNIX)
ENDIF (WIN32)

INCLUDE(strict_warning)

include(FindThreads)
include(enable_qt)
include(qt4_wrap_ts)
include(onyx_test)
include(misc)
include(tc)

LINK_DIRECTORIES(${QT_LIBRARY_DIR})

ENABLE_QT()

QT4_WRAP_CPP(MOC_SCRIBBLEAREA_SRCS scribblearea.h)
ADD_EXECUTABLE(scribble scribble.cpp scribblearea.h scribblearea.cpp
    ${MOC_SCRIBBLEAREA_SRCS})
TARGET_LINK_LIBRARIES(scribble onyx_screen onyxapp onyx_sys onyx_wpa onyx_data ${QT_LIBRARIES})
