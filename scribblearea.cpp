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
    //onyx::screen::watcher().addWatcher(this);
#else
    buffer = QImage(size(), QImage::Format_RGB32);
    painter.begin(&buffer);
    painter.eraseRect(QRect(QPoint(0, 0), size()));
    painter.end();
#endif
}

void ScribbleArea::resizeEvent(QResizeEvent *)
{
#ifndef BUILD_FOR_ARM
    buffer = QImage(size(), QImage::Format_RGB32);
    painter.begin(&buffer);
    painter.eraseRect(QRect(QPoint(0, 0), size()));
    painter.end();
#endif
}

void ScribbleArea::redrawPage(const ScribblePage &page, int layer)
{
#ifndef BUILD_FOR_ARM
    buffer = QImage(size(), QImage::Format_RGB32);
    painter.begin(&buffer);

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.eraseRect(QRect(QPoint(0, 0), size()));
#else

    /*
    onyx::screen::instance().updateWidgetRegion(this, QRect(mapToGlobal(QPoint(0, 0)), size()),
                                                onyx::screen::ScreenProxy::GC, true,
                                                onyx::screen::ScreenCommand::WAIT_ALL);
    */
    /* TODO this does not clear the screen. Does it perhaps update the Qt buffer from the device buffer? */
    onyx::screen::instance().fillScreen(0xff);
    onyx::screen::instance().ensureUpdateFinished();
    onyx::screen::instance().updateWidgetRegion(0, QRect(0, 0, 500, 500));
    /* TODO If this works, try updateWidget again */
#endif

    for (int li = 0; li <= layer; li ++) {
        const ScribbleLayer &l = page.layers[li];
        foreach (const ScribbleStroke &s, l.items) {
            drawStroke(s);
        }
    }
    /* TODO see if this works */
    onyx::screen::instance().updateWidgetRegion(0, QRect(0, 0, 500, 500));
#ifndef BUILD_FOR_ARM
    painter.end();
    update();
#endif
}

void ScribbleArea::drawStroke(const ScribbleStroke &s, bool unpaint)
{
#ifdef BUILD_FOR_ARM
    /* TODO check if drawing multiple lines works */
    for (int i = 0; i + 1 < s.getPoints().size(); i ++) {
        QVector<QPoint> line;
        line.append(mapToGlobal(s.getPoints()[i].toPoint()));
        line.append(mapToGlobal(s.getPoints()[i + 1].toPoint()));
        int color = unpaint ? 0xff : 0x00;
        /* TODO size - could be arbitrary integer - it seems that unpainting
         * does not work with size 1 */
        onyx::screen::instance().drawLines(line.data(), 2, color, 2);
    }
#else
    QPen pen = s.getPen();
    pen.setWidth(2); /* simulation */
    if (unpaint) {
        /* TODO does not work very well. antialiasing? */
        pen.setColor(QColor(0xff, 0xff, 0xff));
    }
    painter.setPen(pen);
    painter.drawPolyline(s.getPoints());
#endif
}

void ScribbleArea::drawStrokeSegment(const ScribbleStroke &s, int i, bool unpaint)
{
#ifdef BUILD_FOR_ARM
    QVector<QPoint> line;
    line.append(mapToGlobal(s.getPoints()[i].toPoint()));
    line.append(mapToGlobal(s.getPoints()[i + 1].toPoint()));
    int color = unpaint ? 0xff : 0x00;
    /* TODO size - could be arbitrary integer - it seems that unpainting
     * does not work with size 1 */
    onyx::screen::instance().drawLines(line.data(), 2, color, 2);
#else
    QPen pen = s.getPen();
    pen.setWidth(2); /* simulation */
    if (unpaint) {
        /* TODO does not work very well. antialiasing? */
        pen.setColor(QColor(0xff, 0xff, 0xff));
    }
    painter.setPen(pen);
    painter.drawLine(s.getPoints()[i], s.getPoints()[i + 1]);
#endif

}

void ScribbleArea::drawLastStrokeSegment(const ScribbleStroke &s)
{
    int n = s.getPoints().size();
    if (n < 2) return;

#ifndef BUILD_FOR_ARM
    painter.begin(&buffer);
#endif

    drawStrokeSegment(s, n - 2);

#ifndef BUILD_FOR_ARM
    painter.end();
    update();
#endif
}

void ScribbleArea::drawCompletedStroke(const ScribbleStroke &)
{
    /* TODO here we could redraw it nicely */
}

void ScribbleArea::updateStrokes(const ScribblePage &page, int layer, const QList<ScribbleStroke> &removedStrokes)
{
    /* this will not work if there is a background or if there are strokes of different colors */

    /* repaint all removed strokes white */
#ifndef BUILD_FOR_ARM
    painter.begin(&buffer);
    painter.setRenderHint(QPainter::Antialiasing, false);
#endif
    for (int i = 0; i < removedStrokes.length(); i ++) {
        drawStroke(removedStrokes[i], true);
    }

    /* find all strokes intersecting with the removed strokes and repaint them */
    /* perhaps first use a combined bounding rect for all removed strokes? */
    for (int i = 0; i <= layer; i ++) {
        const ScribbleLayer &l = page.layers[i];
        for (int j = 0; j < l.items.length(); j ++) {
            const ScribbleStroke &s = l.items[j];
            for (int k = 0; k < removedStrokes.length(); k ++) {
                const ScribbleStroke &sr = removedStrokes[k];
                /* TODO we only need to redraw intersecting line segments */
                if (s.boundingRectIntersects(sr)) {
                    for (int p = 0; p < s.getPoints().size() - 1; p ++) {
                        if (s.segmentIntersects(p, sr))
                            drawStrokeSegment(s, p);
                    }
                }
            }
        }
    }
#ifndef BUILD_FOR_ARM
    painter.end();
    update();
#endif

}

void ScribbleArea::paintEvent(QPaintEvent *)
{
#ifndef BUILD_FOR_ARM
    QPainter bufferPainter(this);
    bufferPainter.drawImage(QPoint(0,0), buffer);
#endif
}

