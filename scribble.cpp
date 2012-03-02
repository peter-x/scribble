/*
 * scribble: Scribbling Application for Onyx Boox M92
 *
 * Copyright (C) 2012 peter-x
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
