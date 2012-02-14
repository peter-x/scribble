#include "scribblearea.h"

#include <QPainter>
#include <QPen>
#include <QMouseEvent>

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

static const QString SCRIBBLE_PATH = "scribble_doc";

ScribbleArea::ScribbleArea(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint)
{
    setMinimumSize(100, 100);
    setAutoFillBackground(true); /* TODO correct? */
    setBackgroundRole(QPalette::Base);

#ifdef BUILD_FOR_ARM
    onyx::screen::watcher().addWatcher(this);
#else
    /* TODO is width already available? */
    buffer = QImage(size(), QImage::Format_RGB32);
    QPainter painter(&buffer);
    painter.eraseRect(QRect(QPoint(0, 0), size()));
#endif
}

void ScribbleArea::resizeEvent(QResizeEvent *)
{
    buffer = QImage(size(), QImage::Format_RGB32);
    QPainter painter(&buffer);
    painter.eraseRect(QRect(QPoint(0, 0), size()));
}

void ScribbleArea::redrawPage(const ScribblePage &page, int layer)
{
    buffer = QImage(size(), QImage::Format_RGB32);
    QPainter painter(&buffer);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.eraseRect(QRect(QPoint(0, 0), size()));
    for (int li = 0; li <= layer; li ++) {
        const ScribbleLayer &l = page.layers[li];
        foreach (const ScribbleStroke &s, l.items) {
            painter.setPen(s.pen);
            painter.drawPolyline(s.points);
        }
    }
    update();
}

void ScribbleArea::drawStrokePoint(const ScribbleStroke &s)
{
    if (s.points.size() < 2) return;

    QPainter painter(&buffer);

    int n = s.points.size();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawLine(s.points[n - 2], s.points[n - 1]);
    update();
}

void ScribbleArea::drawCompletedStroke(const ScribbleStroke &)
{
    /* TODO here we could redraw it nicely */
}

void ScribbleArea::updateStrokesInRegion(const ScribblePage &page, int layer, QRectF boundingBox)
{
    /* TODO we could optimize it */
    redrawPage(page, layer);
}



void ScribbleArea::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(QPoint(0,0), buffer);
}

