#include "mainwidget.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent)
{
    document = new ScribbleDocument(this);

    scribbleArea = new ScribbleArea(this);

    QToolBar *toolbar = new QToolBar(this);
    /* TODO use action groups */

    QAction *left = new QAction(QIcon(":/images/left_arrow.png"),
                                "left", this);
    toolbar->addAction(left);

    QAction *pen = new QAction(QIcon(":images/sketch_mode_sketch.png"),
                               "pen", this);
    QAction *eraser = new QAction(QIcon(":images/sketch_mode_erase.png"),
                               "eraser", this);

    connect(pen, SIGNAL(triggered()), this, SLOT(usePen()));
    toolbar->addAction(pen);
    connect(eraser, SIGNAL(triggered()), this, SLOT(useEraser()));
    toolbar->addAction(eraser);
    toolbar->addAction("thin");
    toolbar->addAction("thick");
    toolbar->addAction("extra thick");
    toolbar->addAction("right");

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
    scribbleArea->setPageLayer(document->pages.length() - 1,
                               document->pages.last().layers.length() - 1);
}

ScribblePage *MainWidget::getPage(int num)
{
    if (num < 0 || num >= document->pages.length())
        return 0;
    else
        return &(document->pages[num]);
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

void MainWidget::usePen()
{
    scribbleArea->setModeSizeColor(ScribbleArea::PEN, 3, QColor("#000000"));
}

void MainWidget::useEraser()
{
    scribbleArea->setModeSizeColor(ScribbleArea::ERASER, 8, QColor("#000000"));
}

void MainWidget::save()
{

}
