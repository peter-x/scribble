#include "mainwidget.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent)
{
    document = new ScribbleDocument(this);
    scribbleArea = new ScribbleArea(this);

    connect(document, SIGNAL(pageOrLayerChanged(ScribblePage,int)), scribbleArea, SLOT(redrawPage(ScribblePage,int)));
    connect(document, SIGNAL(strokePointAdded(ScribbleStroke)), scribbleArea, SLOT(drawStrokePoint(ScribbleStroke)));
    connect(document, SIGNAL(strokeCompleted(ScribbleStroke)), scribbleArea, SLOT(drawCompletedStroke(ScribbleStroke)));
    connect(document, SIGNAL(strokesChanged(ScribblePage,int,QRectF)), scribbleArea, SLOT(updateStrokesInRegion(ScribblePage,int,QRectF)));

    connect(&touchListener, SIGNAL(touchData(TouchData &)), this, SLOT(touchEventDataReceived(TouchData &)));

    QToolBar *toolbar = new QToolBar(this);
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

    QVBoxLayout *layout = new QVBoxLayout;

    layout->addWidget(toolbar);
    layout->addWidget(scribbleArea);

    setLayout(layout);
    onyx::screen::watcher().addWatcher(this);

    QTimer *save_timer = new QTimer(this);
    connect(save_timer, SIGNAL(timeout()), SLOT(save()));
    /* save every 5 seconds */
    save_timer->start(5000);
}

void MainWidget::loadFile(const QFile &file)
{
    /* TODO check return value */
    document->loadXournalFile(file);
}

void MainWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
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
    if (pressure_of_last_point_ == 0 && touch_point.pressure > 0)
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

void MainWidget::save()
{

}
