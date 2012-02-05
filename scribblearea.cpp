#include "scribblearea.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

static const QString SCRIBBLE_PATH = "scribble_doc";

ScribbleArea::ScribbleArea(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint), sketchProxy(0), currentPage(0)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    onyx::screen::watcher().addWatcher(this);

    sketchProxy = new sketch::SketchProxy();
    sketchProxy->setMode(ui::MODE_SKETCHING);
    sketchProxy->attachWidget(this);

    updateSketchProxy();
}

void ScribbleArea::updateSketchProxy()
{
    sketchProxy->deactivateAll();

    // update zoom factor
    sketchProxy->setZoom(ZOOM_ACTUAL);
    sketchProxy->setContentOrient(ROTATE_0_DEGREE);
    sketchProxy->setWidgetOrient(ROTATE_0_DEGREE /*getSystemRotateDegree()*/);

    // draw sketches in this page
    // the page number of any image is 0
    sketch::PageKey page_key;
    page_key.setNum(0);

    QRect page_area(QPoint(0, 0), size());

    sketchProxy->activatePage(SCRIBBLE_PATH, page_key);
    sketchProxy->updatePageDisplayRegion(SCRIBBLE_PATH, page_key, page_area);
}


void ScribbleArea::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (currentPage == 0) return;

    painter.setRenderHint(QPainter::Antialiasing);
    foreach (const ScribbleLayer &l, currentPage->layers) {
        foreach (const ScribbleStroke &s, l.items) {
            painter.setPen(s.pen);
            painter.drawPolyline(s.points);
        }
    }

#if 0
    // update zoom factor
    sketchProxy->setZoom(ZOOM_ACTUAL);
    sketchProxy->setContentOrient(ROTATE_0_DEGREE);
    sketchProxy->setWidgetOrient(ROTATE_0_DEGREE /*getSystemRotateDegree()*/);

    // draw sketches in this page
    // the page number of any image is 0
    sketch::PageKey page_key;
    page_key.setNum(0);
    QRect page_area(QPoint(0, 0), size());

    sketchProxy->updatePageDisplayRegion(SCRIBBLE_PATH, page_key, page_area);
    sketchProxy->paintPage(SCRIBBLE_PATH, page_key, painter);
#endif
}

bool ScribbleArea::event(QEvent *e)
{
    switch (e->type())
    {
    case QEvent::HoverMove:
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
        e->accept();
        return true;
    default:
        break;
    }
    return QWidget::event(e);
}

void ScribbleArea::setModeShapeColor(const ui::SketchMode m,
                                     const ui::SketchShape s,
                                     const ui::SketchColor c)
{
    sketchProxy->setMode(m);
    sketchProxy->setShape(s);
    sketchProxy->setColor(c);
}

void ScribbleArea::renderPage(ScribblePage *page)
{
    /* TODO dangerous, because it can be freed */
    currentPage = page;
    update();
}
