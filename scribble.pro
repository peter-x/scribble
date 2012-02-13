QT += core network sql gui

CONFIG += debug
CONFIG -= release

SOURCES += scribble.cpp \
    mainwidget.cpp \
    scribblearea.cpp \
    scribble_document.cpp

LIBS += -L /usr/local/lib -lz -lonyxapp -lonyx_base -lonyx_ui -lonyx_screen -lonyx_sys -lonyx_wpa -lonyx_wireless -lonyx_data -lonyx_touch -lonyx_cms

INCLUDEPATH += /opt/onyx/arm/iclude

HEADERS += \
    mainwidget.h \
    scribblearea.h \
    scribble_document.h

RESOURCES +=
