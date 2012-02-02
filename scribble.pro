QT += core network dbus sql gui
SOURCES += scribble.cpp \
    mainwidget.cpp \
    scribblearea.cpp

LIBS += -L /usr/local/lib -lonyxapp -lonyx_base -lonyx_ui -lonyx_screen -lonyx_sys -lonyx_wpa -lonyx_wireless -lonyx_data -lonyx_touch -lonyx_cms

INCLUDEPATH += /opt/onyx/arm/iclude

HEADERS += \
    mainwidget.h \
    scribblearea.h
