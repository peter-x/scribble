// -*- mode: c++; c-basic-offset: 4; -*-

#include <QtGui/QtGui>

#include "onyx/application.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/sys/sys_status.h"

#include "scribblearea.h"

class MyApplication : public onyx::Application {
  public:
    MyApplication() : mainWidget(0), toolbar(0)  {}

    virtual ~MyApplication() {
        delete toolbar;
        delete mainWidget;
    }

    virtual int exec() {
        sys::SysStatus::instance().setSystemBusy(false);

        mainWidget = new QWidget();

        toolbar = new QToolBar();

        toolbar->addAction("left");
        toolbar->addAction("pen");
        toolbar->addAction("eraser");
        toolbar->addAction("thin");
        toolbar->addAction("thick");
        toolbar->addAction("extra thick");
        toolbar->addAction("right");

        QVBoxLayout *layout = new QVBoxLayout;

        layout->addWidget(toolbar);

        mainWidget->setLayout(layout);
        mainWidget->showFullScreen();
        onyx::screen::watcher().addWatcher(mainWidget);

        return 0;
    }
  private:
    QWidget *mainWidget;
    QToolBar *toolbar;
};

ONYX_APP_CLASS(MyApplication)
