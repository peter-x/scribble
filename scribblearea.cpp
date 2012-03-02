#include "scribblearea.h"

#include <QPainter>
#include <QPen>
#include <QMouseEvent>

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

void ScribbleGraphicsContext::drawPage(const ScribblePage &page, int maxLayer)
{
    for (int li = 0; li <= maxLayer; li ++) {
        const ScribbleLayer &l = page.layers[li];
        foreach (const ScribbleStroke &s, l.items) {
            drawStroke(s);
        }
    }
}

void ScribbleGraphicsContext::drawStroke(const ScribbleStroke &stroke)
{
    QPen pen = stroke.getPen();
    unsigned char color = undraw ? 0xff : pen.color().lightness();

    const QPolygonF &points = stroke.getPoints();
    if (painter) {
        for (int i = 0; i + 1 < points.size(); i ++) {
            drawLinePainter(points[i].toPoint(), points[i + 1].toPoint(), color, qCeil(pen.widthF()));
        }
    } else {
        /* TODO check if drawing multiple lines works */
        for (int i = 0; i + 1 < points.size(); i ++) {
            drawLineDirect(points[i].toPoint(), points[i + 1].toPoint(), color, qCeil(pen.widthF()));
        }
    }
}

void ScribbleGraphicsContext::drawStrokeSegment(const ScribbleStroke &stroke, int i)
{
    QPen pen = stroke.getPen();
    unsigned char color = undraw ? 0xff : pen.color().lightness();
    const QPolygonF &points = stroke.getPoints();
    if (painter) {
        drawLinePainter(points[i].toPoint(), points[i + 1].toPoint(), color, qCeil(pen.widthF()));
    } else {
        drawLineDirect(points[i].toPoint(), points[i + 1].toPoint(), color, qCeil(pen.widthF()));
    }
}

void ScribbleGraphicsContext::drawLinePainter(const QPoint &p1, const QPoint &p2, unsigned char color, int width)
{
    QBrush brush(QColor(color, color, color), Qt::SolidPattern);

    int rad = width / 2;
    int px1 = p1.x() - rad;
    int py1 = p1.y() - rad;
    int px2 = p2.x() - rad;
    int py2 = p2.y() - rad;

    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillRect(px2, py2, width, width, brush);

    bool is_steep = qAbs(py2 - py1) > qAbs(px2 - px1);
    if (is_steep) {
        std::swap(px1, py1);
        std::swap(px2, py2);
    }

    // setup line draw
    int deltax   = qAbs(px2 - px1);
    int deltaerr = qAbs(py2 - py1);
    int error = 0;
    int x = px1;
    int y = py1;

    // setup step increment
    int xstep = (px1 < px2) ? 1 : -1;
    int ystep = (py1 < py2) ? 1 : -1;

    if (is_steep) {
        for (int numpix = 0; numpix < deltax; numpix++) {
            x += xstep;
            error += deltaerr;

            if (2 * error > deltax) {
                y += ystep;
                error -= deltax;
            }

            painter->fillRect(y, x, width, width, brush);
        }
    } else {
        for (int numpix = 0; numpix < deltax; numpix++) {
            x += xstep;
            error += deltaerr;

            if (2 * error > deltax) {
                y += ystep;
                error -= deltax;
            }

            painter->fillRect(x, y, width, width, brush);
        }
    }

}

void ScribbleGraphicsContext::drawLineDirect(const QPoint &p1, const QPoint &p2, unsigned char color, int width)
{
    QVector<QPoint> line;
    line.append(widget->mapToGlobal(p1));
    line.append(widget->mapToGlobal(p2));
    /* TODO try if intermediate colors work */
    color = color >= 0x7f ? 0xff : 0x00;
    /* TODO width smller than two does not work */
    if (width < 2) width = 2;
    onyx::screen::instance().drawLines(line.data(), 2, color, width);
}

ScribbleArea::ScribbleArea(QWidget *parent, const ScribbleDocument *document) :
    QWidget(parent, Qt::FramelessWindowHint), document(document)
{
    setMinimumSize(100, 100);
    setAutoFillBackground(false);
    setBackgroundRole(QPalette::Base);

#ifdef BUILD_FOR_ARM
    onyx::screen::watcher().addWatcher(this);
#endif
    buffer = QImage(size(), QImage::Format_Mono); /* TODO this is BW and not monochrome */
    QPainter painter(&buffer);
    painter.eraseRect(rect());

    regionToUpdate = QRegion(rect());
    updateTimer.setInterval(80);
    connect(&updateTimer, SIGNAL(timeout()), SLOT(updateIfNeeded()));
    updateTimer.start();

    connect(document, SIGNAL(pageOrLayerChanged(ScribblePage,int)), SLOT(redrawPage(ScribblePage,int)));
    connect(document, SIGNAL(strokePointAdded(ScribbleStroke)), SLOT(drawLastStrokeSegment(ScribbleStroke)));
    connect(document, SIGNAL(strokeCompleted(ScribbleStroke)), SLOT(drawCompletedStroke(ScribbleStroke)));
    connect(document, SIGNAL(strokesChanged(ScribblePage,int,QList<ScribbleStroke>)), SLOT(updateStrokes(ScribblePage,int,QList<ScribbleStroke>)));
}

void ScribbleArea::resizeEvent(QResizeEvent *ev)
{
    emit resized(ev->size());
    redrawPage(document->getCurrentPage(), document->getCurrentLayer());
}

void ScribbleArea::redrawPage(const ScribblePage &page, int layer)
{
    buffer = QImage(size(), QImage::Format_Mono);
    QPainter painter(&buffer);
    painter.eraseRect(rect());

    ScribbleGraphicsContext ctx(&painter, false);
    ctx.drawPage(page, layer);

    regionToUpdate = rect();
}

void ScribbleArea::drawLastStrokeSegment(const ScribbleStroke &s)
{
    int n = s.getPoints().size();
    if (n < 2) return;

#if defined(BUILD_FOR_ARM)
    ScribbleGraphicsContext ctx(this, false);
#else
    QPainter painter(&buffer);
    ScribbleGraphicsContext ctx(&painter, false);
#endif

    ctx.drawStrokeSegment(s, n - 2);

#if !defined(BUILD_FOR_ARM)
    qreal width = s.getPen().widthF();
    QPointF p1 = s.getPoints()[n - 2];
    QPointF p2 = s.getPoints()[n - 1];
    QRect br(QPoint(qFloor(qMin(p1.x(), p2.x()) - width / 2.0) - 1,
                    qFloor(qMin(p1.y(), p2.y()) - width / 2.0) - 1),
             QSize(qCeil(qAbs(p1.x() - p2.x()) + width) + 2,
                   qCeil(qAbs(p1.y() - p2.y()) + width) + 2));

    regionToUpdate += br;
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
#if defined(BUILD_FOR_ARM)
    ScribbleGraphicsContext ctx(this, true);
#else
    QPainter painter(&buffer);
    ScribbleGraphicsContext ctx(&painter, true);
#endif
    foreach (const ScribbleStroke &s, removedStrokes) {
        ctx.drawStroke(s);
#if !defined(BUILD_FOR_ARM)
        /* TODO round in the right direction */
        regionToUpdate += s.getBoundingRect().toRect();
#endif
    }

#if 0

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
#endif
}

void ScribbleArea::updateIfNeeded()
{
    if (!regionToUpdate.isEmpty()) {
        update(regionToUpdate);
    }
}

void ScribbleArea::paintEvent(QPaintEvent *ev)
{
    QPainter bufferPainter(this);
    bufferPainter.drawImage(QPoint(), buffer);
    regionToUpdate = QRect();
#if defined(BUILD_FOR_ARM)
    /* TODO we could safely request to update the whole rect */
    onyx::screen::watcher().enqueue(this, ev->rect(), onyx::screen::ScreenProxy::DW);
    //onyx::screen::instance().updateWidgetRegion(this, ev->rect(), onyx::screen::ScreenProxy::DW, false);
#endif
}

