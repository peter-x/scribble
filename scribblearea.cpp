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
#ifdef BUILD_FOR_ARM
    setAutoFillBackground(false); /* TODO correct? */
#else
    setAutoFillBackground(true);
#endif
    setBackgroundRole(QPalette::Base);

#ifdef BUILD_FOR_ARM
    onyx::screen::watcher().addWatcher(this);
#else
    buffer = QImage(size(), QImage::Format_RGB32);
    QPainter painter(&buffer);
    painter.eraseRect(QRect(QPoint(0, 0), size()));
#endif
}

void ScribbleArea::resizeEvent(QResizeEvent *)
{
#ifndef BUILD_FOR_ARM
    buffer = QImage(size(), QImage::Format_RGB32);
    QPainter painter(&buffer);
    painter.eraseRect(QRect(QPoint(0, 0), size()));
#endif
}

void ScribbleArea::redrawPage(const ScribblePage &page, int layer)
{
#ifndef BUILD_FOR_ARM
    buffer = QImage(size(), QImage::Format_RGB32);
    QPainter painter(&buffer);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.eraseRect(QRect(QPoint(0, 0), size()));
#endif
    /* TODO eink: erase screen - perhaps just sync double buffer? */
    for (int li = 0; li <= layer; li ++) {
        const ScribbleLayer &l = page.layers[li];
        foreach (const ScribbleStroke &s, l.items) {
#ifdef BUILD_FOR_ARM
            /* TODO check if drawing multiple lines works */
            for (int i = 0; i + 1 < s.points.size(); i ++) {
                QVector<QPoint> line;
                line.append(mapToGlobal(s.points[i].toPoint()));
                line.append(mapToGlobal(s.points[i + 1].toPoint()));
                onyx::screen::instance().drawLines(line.data(), 2, 0x00, 1);
            }
#else
            painter.setPen(s.pen);
            painter.drawPolyline(s.points);
#endif
        }
    }
#ifndef BUILD_FOR_ARM
    update();
#endif
}

void ScribbleArea::drawStrokePoint(const ScribbleStroke &s)
{
    int n = s.points.size();
    if (n < 2) return;

#ifdef BUILD_FOR_ARM
    QVector<QPoint> line;
    line.append(mapToGlobal(s.points[n - 2].toPoint()));
    line.append(mapToGlobal(s.points[n - 1].toPoint()));
    onyx::screen::instance().drawLines(line.data(), 2, 0x00, 1);
#else
    QPainter painter(&buffer);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawLine(s.points[n - 2], s.points[n - 1]);
    update();
#endif
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
#ifndef BUILD_FOR_ARM
    QPainter painter(this);
    painter.drawImage(QPoint(0,0), buffer);
#endif
}

