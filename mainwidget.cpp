#include "mainwidget.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent)
{
    document = new ScribbleDocument(this);

    /* TODO check ownership transfer and destructor */
    QToolBar *toolbar = new QToolBar(this);
    scribbleArea = new ScribbleArea(this);

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
    document->loadXournalFile(file);
    if (!document->pages.isEmpty())
        scribbleArea->renderPage(&(document->pages[0]));
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
    scribbleArea->setModeShapeColor(ui::MODE_SKETCHING,
                                    ui::SKETCH_SHAPE_0,
                                    ui::SKETCH_COLOR_BLACK);
}

void MainWidget::useEraser()
{
    /* TODO real erase does not seem to work */
    scribbleArea->setModeShapeColor(ui::MODE_SKETCHING,
                                    ui::SKETCH_SHAPE_4,
                                    ui::SKETCH_COLOR_WHITE);

}

void MainWidget::save()
{

}
