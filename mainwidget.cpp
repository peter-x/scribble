#include "mainwidget.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent)
{
    /* TODO check ownership transfer and destructor */
    QToolBar *toolbar = new QToolBar(this);
    scribbleArea = new ScribbleArea(this);

    toolbar->addAction("left");
    toolbar->addAction("pen");
    toolbar->addAction("eraser");
    toolbar->addAction("thin");
    toolbar->addAction("thick");
    toolbar->addAction("extra thick");
    toolbar->addAction("right");

    QVBoxLayout *layout = new QVBoxLayout;

    layout->addWidget(toolbar);
    layout->addWidget(scribbleArea);

    setLayout(layout);
    onyx::screen::watcher().addWatcher(this);
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
