#include "mainwidget.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

#include "onyx/ui/toolbar.h"
#include "onyx/ui/status_bar.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint), currentFile("")
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

    layout->addWidget(toolbar);
    layout->addWidget(scribbleArea);
    layout->addWidget(statusBar);

    setLayout(layout);
    onyx::screen::watcher().addWatcher(this);

    connect(document, SIGNAL(pageOrLayerNumberChanged(int,int,int,int)), SLOT(updateProgressBar(int,int,int,int)));
    connect(statusBar, SIGNAL(progressClicked(int,int)), SLOT(setPage(int,int)));

    connect(&touchListener, SIGNAL(touchData(TouchData &)), this, SLOT(touchEventDataReceived(TouchData &)));

    QTimer *save_timer = new QTimer(this);
    connect(save_timer, SIGNAL(timeout()), SLOT(save()));
    /* save every 5 seconds */
    save_timer->start(5000);
}

void MainWidget::loadFile(const QFile &file)
{
    /* TODO error message */
    if (document->loadXournalFile(file)) {
        currentFile.setFileName(file.fileName());
    }
}

void MainWidget::saveFile(const QFile &file)
{
    document->saveXournalFile(file);
    currentFile.setFileName(file.fileName());
}

void MainWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        save();
        qApp->exit();
    default:
        QWidget::keyPressEvent(event);
    }
}

void MainWidget::touchEventDataReceived(TouchData &data)
{
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

void MainWidget::updateProgressBar(int currentPage, int maxPages, int currentLayer, int maxLayers)
{
    statusBar->setProgress(currentPage + 1, maxPages);
}

void MainWidget::setPage(int percentage, int page)
{
    document->setCurrentPage(page - 1);
}

void MainWidget::save()
{
    if (currentFile.fileName() != "")
        document->saveXournalFile(currentFile);
}
