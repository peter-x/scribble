// -*- mode: c++; c-basic-offset: 4; -*-

#include <QtGui/QtGui>

#include "onyx/application.h"
#include "onyx/sys/sys_status.h"

#include "mainwidget.h"

class MyApplication : public onyx::Application {
  public:
    MyApplication() : mainWidget(0)  {}

    virtual ~MyApplication() {
        delete mainWidget;
    }

    virtual int exec() {
        Q_INIT_RESOURCE(onyx_ui_images);
        sys::SysStatus::instance().setSystemBusy(false);

        mainWidget = new MainWidget();
        QFile file;
        if (QDir("/media/flash").exists()) {
            file.setFileName("/media/flash/scribble.xoj");
        } else {
            file.setFileName(QDir::homePath() + "/scribble.xoj");
        }
        mainWidget->loadFile(file);
        mainWidget->saveFile(file);
        mainWidget->showFullScreen();

        return 0;
    }
  private:
    MainWidget *mainWidget;
};

ONYX_APP_CLASS(MyApplication)
