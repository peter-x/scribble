QT += core network sql gui

SOURCES += scribble.cpp \
    mainwidget.cpp \
    scribblearea.cpp \
    scribble_document.cpp \
    filebrowser.cpp \
    tree_view.cpp

LIBS += -L /usr/local/lib -lz -lonyxapp -lonyx_base -lonyx_ui -lonyx_screen -lonyx_sys -lonyx_wpa -lonyx_wireless -lonyx_data -lonyx_touch -lonyx_cms

INCLUDEPATH += /opt/onyx/arm/include

HEADERS += \
    mainwidget.h \
    scribblearea.h \
    scribble_document.h \
    filebrowser.h \
    tree_view.h

RESOURCES +=
