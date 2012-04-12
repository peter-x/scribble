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

#include "mainwidget.h"

#include "filebrowser.h"
#include "fileio.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

#include "onyx/ui/toolbar.h"
#include "onyx/ui/status_bar.h"
#include "onyx/ui/onyx_dialog.h"
#include "onyx/ui/thumbnail_view.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint), touchActive(true), currentFile("")
{
    document = new ScribbleDocument(this);
    scribbleArea = new ScribbleArea(this, document);
    pressure_of_last_point_ = 0;

    ui::OnyxToolBar *toolbar = new ui::OnyxToolBar(this);
    /* TODO use action groups */

    QAction *left = new QAction(QIcon(":/images/left_arrow.png"),
                                "previous page", this);
    connect(left, SIGNAL(triggered()), document, SLOT(previousPage()));
    toolbar->addAction(left);

    QAction *pen = new QAction(QIcon(":images/sketch_mode_sketch.png"),
                               "pen", this);
    connect(pen, SIGNAL(triggered()), document, SLOT(usePen()));
    toolbar->addAction(pen);

    QAction *eraser = new QAction(QIcon(":images/sketch_mode_erase.png"),
                               "eraser", this);
    connect(eraser, SIGNAL(triggered()), document, SLOT(useEraser()));
    toolbar->addAction(eraser);

    /*
    QAction *thin = new QAction(QIcon(":images/sketch_shape_1.png"),
                               "thin", this);
    connect(thin, SIGNAL(triggered()), this, SLOT(sizeThin()));
    toolbar->addAction(thin);

    QAction *medium = new QAction(QIcon(":images/sketch_shape_3.png"),
                               "thin", this);
    connect(medium, SIGNAL(triggered()), this, SLOT(sizeMedium()));
    toolbar->addAction(medium);

    QAction *thick = new QAction(QIcon(":images/sketch_shape_5.png"),
                               "thin", this);
    connect(thick, SIGNAL(triggered()), this, SLOT(sizeThick()));
    toolbar->addAction(thick);
    */

    /* TODO icon at least looks like "open" */
    QAction *open = new QAction(QIcon(":/images/config.png"),
                                "open document", this);
    connect(open, SIGNAL(triggered()), SLOT(open()));
    toolbar->addAction(open);

    /*
    QAction *save = new QAction(QIcon(":/images/save_document.png"),
                                "save as", this);
    connect(save, SIGNAL(triggered()), SLOT(saveAs()));
    toolbar->addAction(save);
    */

    QAction *right = new QAction(QIcon(":/images/right_arrow.png"),
                                "next page", this);
    connect(right, SIGNAL(triggered()), document, SLOT(nextPage()));
    toolbar->addAction(right);

    /* TODO need UI methods for:
     * layer access
     * loading and saving
     * undo
     * set page and layer
     * remove page, insert page
     * set background
     * some settings
     */

    statusBar = new ui::StatusBar(this, ui::MENU | ui::PROGRESS |
                                                        ui::BATTERY | ui::SCREEN_REFRESH |
                                                        ui::CLOCK);

    QVBoxLayout *layout = new QVBoxLayout;

    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addWidget(scribbleArea);
    layout->addWidget(statusBar);

    setLayout(layout);
    onyx::screen::watcher().addWatcher(this);

    asyncWriter = new AsyncWriter(this);

    connect(document, SIGNAL(pageOrLayerNumberChanged(int,int,int,int)), SLOT(updateProgressBar(int,int,int,int)));
    connect(scribbleArea, SIGNAL(resized(QSize)), document, SLOT(setViewSize(QSize)));
    connect(statusBar, SIGNAL(progressClicked(int,int)), SLOT(setPage(int,int)));

    connect(&touchListener, SIGNAL(touchData(TouchData &)), this, SLOT(touchEventDataReceived(TouchData &)));

    QTimer *save_timer = new QTimer(this);
    connect(save_timer, SIGNAL(timeout()), SLOT(saveAsynchronously()));
    /* save every 5 seconds */
    save_timer->start(5000);
}

void MainWidget::loadFile(const QFile &file)
{
    save();

    /* TODO error message */
    QByteArray data = FileIO::readGZFileLocked(file);
    if (document->loadXournalFile(data)) {
        currentFile.setFileName(file.fileName());
    }
}

void MainWidget::saveFile(const QFile &file)
{
    FileIO::writeGZFileLocked(file, document->toXournalXMLFormat());
    currentFile.setFileName(file.fileName());
}

void MainWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        save();
        qApp->exit();
    case Qt::Key_Right:
    case Qt::Key_PageDown:
        document->nextPage();
        break;
    case Qt::Key_Left:
    case Qt::Key_PageUp:
        document->previousPage();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void MainWidget::touchEventDataReceived(TouchData &data)
{
    if (!touchActive) return;

    // get widget pos
    OnyxTouchPoint &touch_point = data.points[0];
    QPoint global_pos(touch_point.x, touch_point.y);
    QPoint widget_pos = scribbleArea->mapFromGlobal(global_pos);

    // check whether the point is in widget
    if (widget_pos.x() < 0 || widget_pos.y() < 0 ||
        widget_pos.x() > width() || widget_pos.y() > height())
    {
        // qDebug("Out of boundary");
        return;
    }

    // construct a mouse event
    QEvent::Type type = QEvent::MouseMove;
    if (pressure_of_last_point_ <= 0 && touch_point.pressure > 0)
        type = QEvent::MouseButtonPress;
    if (pressure_of_last_point_ > 0 && touch_point.pressure <= 0)
        type = QEvent::MouseButtonRelease;
    /* TODO can we adjust the size depending on pressure? */

    QMouseEvent me(type, widget_pos, global_pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    switch (type)
    {
    case QEvent::MouseButtonPress:
        document->mousePressEvent(&me);
        break;
    case QEvent::MouseMove:
        document->mouseMoveEvent(&me);
        break;
    case QEvent::MouseButtonRelease:
        document->mouseReleaseEvent(&me);
        break;
    default:
        break;
    }
    pressure_of_last_point_ = touch_point.pressure;
}

void MainWidget::mousePressEvent(QMouseEvent *ev)
{
    TouchData data;
    data.points[0].x = ev->globalX();
    data.points[0].y = ev->globalY();
    data.points[0].pressure = ev->buttons() & Qt::LeftButton;
    touchEventDataReceived(data);
}

void MainWidget::mouseMoveEvent(QMouseEvent *ev)
{
    TouchData data;
    data.points[0].x = ev->globalX();
    data.points[0].y = ev->globalY();
    data.points[0].pressure = ev->buttons() & Qt::LeftButton;
    touchEventDataReceived(data);
}

void MainWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    TouchData data;
    data.points[0].x = ev->globalX();
    data.points[0].y = ev->globalY();
    data.points[0].pressure = 0;
    touchEventDataReceived(data);
}

void MainWidget::open()
{
    touchActive = false;

    FileBrowser fileBrowser(this);
    /* TODO save last path */
    QString path = fileBrowser.showLoadFile(currentFile.fileName());
    if (path.isEmpty()) {
        touchActive = true;
        return;
    }
    loadFile(QFile(path));

    touchActive = true;
}

void MainWidget::save()
{
    if (!currentFile.fileName().isEmpty()) {
        asyncWriter->stopWriting();
        /* save timeout cannot occur now since this is the same thread */
        saveFile(currentFile);
    }
}

void MainWidget::saveAs()
{
    QString file = QFileDialog::getSaveFileName(this, "Save Scribble File",
                                                currentFile.fileName(),
                                                "Xournal Files (*.xoj);;All Files (*)");
    if (!file.isEmpty()) {
        saveFile(QFile(file));
    }
    /* TODO set current filename */
}

void MainWidget::updateProgressBar(int currentPage, int maxPages, int currentLayer, int maxLayers)
{
    statusBar->setProgress(currentPage + 1, maxPages);
}

void MainWidget::setPage(int percentage, int page)
{
    document->setCurrentPage(page - 1);
}

void MainWidget::saveAsynchronously()
{
    if (!currentFile.fileName().isEmpty() && document->hasChangedSinceLastSave()) {
        document->setSaved();
        asyncWriter->writeData(document->getPagesCopy(), currentFile);
    }
}
