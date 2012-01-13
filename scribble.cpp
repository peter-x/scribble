// -*- mode: c++; c-basic-offset: 4; -*-

#include <QtGui/QtGui>

#include "onyx/application.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/sys/sys_status.h"

#include "scribblearea.h"

class MyApplication : public onyx::Application {
  public:
    MyApplication() : widget_(NULL) {}

    virtual ~MyApplication() {
        delete widget_;
    }

    virtual int exec() {
        sys::SysStatus::instance().setSystemBusy(false); 
        widget_ = new ScribbleArea;
        widget_->showFullScreen();
        widget_->refreshScreen();
        return 0;
    }
  private:
    ScribbleArea* widget_;
};

ONYX_APP_CLASS(MyApplication)
