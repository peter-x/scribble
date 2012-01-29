QT += core network dbus sql
SOURCES += scribble.cpp

LIBS += -L /usr/local/lib -lonyxapp -lonyx_base -lonyx_ui -lonyx_screen -lonyx_sys -lonyx_wpa -lonyx_wireless -lonyx_data

INCLUDEPATH += /opt/onyx/arm/iclude
