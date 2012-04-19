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

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtGui>

#include <onyx/ui/status_bar.h>
#include <onyx/touch/touch_listener.h>

#include "asyncwriter.h"
#include "scribblearea.h"
#include "scribble_document.h"

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = 0);
    void loadFile(const QFile&);
    void saveFile(const QFile&);

signals:
    void saveToGZFileAsynchronously(const QFile &file, const QByteArray &data);

public slots:
    void saveAsynchronously();

private slots:
    void touchEventDataReceived(const TouchData &);
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

    void save();
    void saveAs();
    void open();

    void updateProgressBar(int currentPage, int maxPages, int currentLayer, int maxLayers);
    void setPage(int percentage, int page);


protected:
    void keyPressEvent(QKeyEvent *);

private:
    TouchEventListener touchListener;
    int pressure_of_last_point_;

    bool touchActive;

    AsyncWriter *asyncWriter;
    QFile currentFile;
    ScribbleArea *scribbleArea;
    ScribbleDocument *document;

    ui::StatusBar *statusBar;
};

#endif // MAINWIDGET_H
