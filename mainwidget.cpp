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
                                "previous page", this);
    connect(left, SIGNAL(triggered()), this, SLOT(previousPage()));
    toolbar->addAction(left);

    QAction *pen = new QAction(QIcon(":images/sketch_mode_sketch.png"),
                               "pen", this);
    connect(pen, SIGNAL(triggered()), this, SLOT(usePen()));
    toolbar->addAction(pen);

    QAction *eraser = new QAction(QIcon(":images/sketch_mode_erase.png"),
                               "eraser", this);
    connect(eraser, SIGNAL(triggered()), this, SLOT(useEraser()));
    toolbar->addAction(eraser);

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

    QAction *right = new QAction(QIcon(":/images/right_arrow.png"),
                                "next page", this);
    connect(right, SIGNAL(triggered()), this, SLOT(nextPage()));
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
    currentPage = 0;
    currentLayer = getCurrentPage()->layers.length() - 1;
    scribbleArea->pageChanged(getCurrentPage(), currentLayer);
}

ScribblePage *MainWidget::getPage(int num)
{
    if (num < 0 || num >= document->pages.length())
        return 0;
    else
        return &(document->pages[num]);
}

ScribblePage *MainWidget::getCurrentPage()
{
    if (currentPage < 0 || document == 0 || currentPage >= document->pages.length())
        return 0;
    return &(document->pages[currentPage]);
}

int MainWidget::getCurrentPageNumber()
{
    return currentPage;
}

int MainWidget::getCurrentLayer()
{
    return currentLayer;
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

void MainWidget::updateModeSizeColor()
{
    int size = currentMode == ScribbleArea::PEN ? currentSize : currentSize * 4;
    scribbleArea->setModeSizeColor(currentMode, size, QColor("#000000"));
}

void MainWidget::usePen()
{
    currentMode = ScribbleArea::PEN;
    updateModeSizeColor();
}

void MainWidget::useEraser()
{
    currentMode = ScribbleArea::ERASER;
    updateModeSizeColor();
}

void MainWidget::previousPage()
{
    if (currentPage <= 0) return;
    currentPage -= 1;

    ScribblePage *page = getCurrentPage();
    if (page == 0) {
        currentLayer = 0;
    } else {
        currentLayer = qMin(currentLayer, page->layers.length() - 1);
    }

    scribbleArea->pageChanged(page, currentLayer);
}

void MainWidget::nextPage()
{
    if (document == 0)
        return;
    if (currentPage + 1 >= document->pages.length()) {
        /* create new page */
        ScribblePage p;
        p.layers.append(ScribbleLayer());
        document->pages.append(p);
        currentPage = document->pages.length() - 1;
    } else {
        currentPage += 1;
    }

    ScribblePage *page = getCurrentPage();
    if (page == 0) {
        currentLayer = 0;
    } else {
        currentLayer = qMin(currentLayer, page->layers.length() - 1);
    }
    scribbleArea->pageChanged(page, currentLayer);
}

void MainWidget::sizeThin()
{
    currentSize = 1;
    updateModeSizeColor();
}

void MainWidget::sizeMedium()
{
    currentSize = 2;
    updateModeSizeColor();
}

void MainWidget::sizeThick()
{
    currentSize = 3;
    updateModeSizeColor();
}

void MainWidget::save()
{

}
