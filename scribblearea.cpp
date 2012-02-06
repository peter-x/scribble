#include "scribblearea.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

static const QString SCRIBBLE_PATH = "scribble_doc";

ScribbleArea::ScribbleArea(MainWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint), mainWidget(parent),
    sketching(false)
{
    setMinimumSize(100, 100);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    onyx::screen::watcher().addWatcher(this);
}

void ScribbleArea::setPageLayer(int page, int layer)
{
    currentPage = page;
    currentLayer = layer;
    update();
}

void ScribbleArea::setModeSizeColor(ScribbleMode mode, float size, const QColor &color)
{
    currentMode = mode;
    currentPen.setWidthF(size);
    currentPen.setColor(color);
    /* TODO end current sketch? */
}


void ScribbleArea::paintEvent(QPaintEvent *)
{
    /* TODO only paint event->rect() */
    QPainter painter(this);

    ScribblePage *page = mainWidget->getPage(currentPage);
    if (page == 0) return;

    painter.setRenderHint(QPainter::Antialiasing);
    for (int li = 0; li < page->layers.length(); li ++) {
        const ScribbleLayer &l = page->layers[li];
        foreach (const ScribbleStroke &s, l.items) {
            painter.setPen(s.pen);
            painter.drawPolyline(s.points);
        }
        if (li == currentLayer && sketching && currentMode == PEN) {
            painter.setPen(currentPen);
            painter.drawPolyline(currentStroke);
        }
    }
}

void ScribbleArea::mousePressEvent(QMouseEvent *event)
{
    /* TODO what to do if not empty? */
    currentStroke.clear();
    sketching = true;
    if (currentMode == PEN) {
        currentStroke.append(event->pos());
    } else {
        eraseAt(event->pos());
    }
}

void ScribbleArea::mouseMoveEvent(QMouseEvent *event)
{
    if (!sketching) return;
    if (currentMode == PEN) {
        currentStroke.append(event->pos());
        update();
    } else {
        eraseAt(event->pos());
    }
}

void ScribbleArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (!sketching) return;

    if (currentMode == ERASER) {
        eraseAt(event->pos());
        update();
    } else {
        currentStroke.append(event->pos());
        update();

        ScribbleStroke stroke;
        stroke.pen = currentPen;
        stroke.points = currentStroke;

        currentStroke.clear();

        ScribblePage *page = mainWidget->getPage(currentPage);
        if (page == 0) return;

        /* TODO error if layer does not exist */
        page->layers[currentLayer].items.append(stroke);
    }

    sketching = false;
}

void ScribbleArea::eraseAt(const QPointF &point)
{
    ScribblePage *page = mainWidget->getPage(currentPage);
    if (page == 0) return;

    /* TODO error if layer does not exist */
    ScribbleLayer &layer = page->layers[currentLayer];

    float width = currentPen.widthF();
    float halfWidthSq = width * width / 4.0;
    QRectF eraserBox;
    eraserBox.setSize(QSizeF(width, width));
    eraserBox.moveCenter(point);

    QList<ScribbleStroke> newStrokes;
    for (int i = 0; i < layer.items.length(); i ++) {
        ScribbleStroke &s = layer.items[i];
        /* bounding rect with zero width or height produces not the intended result */
        QRectF bound = s.points.boundingRect();
        bound.adjust(-1, -1, 1, 1);
        if (!bound.intersects(eraserBox))
            continue;

        /* we only check intersections with points, not
         * line segments */
        int retainSequenceStartIndex = 0;
        for (int j = 0; j < s.points.size(); j ++) {
            QPointF p = s.points[j] - point;
            float d = p.x() * p.x() + p.y() * p.y();
            if (d < halfWidthSq) {
                /* remove this point */
                if (retainSequenceStartIndex >= 0) {
                    /* did not remove last point */
                    ScribbleStroke newStroke;
                    newStroke.pen = s.pen;
                    newStroke.points = s.points.mid(retainSequenceStartIndex,
                                                    j - retainSequenceStartIndex);
                    newStrokes.append(newStroke);
                    retainSequenceStartIndex = -1;
                } else {
                    /* also removed last point, do nothing */
                }
            } else {
                /* retain this point */
                if (retainSequenceStartIndex < 0) {
                    /* removed last point */
                    retainSequenceStartIndex = j;
                }
            }
        }
        if (retainSequenceStartIndex == 0) {
            /* no point removed */
            continue;
        }
        if (retainSequenceStartIndex > 0) {
            /* removed some point and have to add remaining points */
            ScribbleStroke newStroke;
            newStroke.pen = s.pen;
            newStroke.points = s.points.mid(retainSequenceStartIndex);
            newStrokes.append(newStroke);
        }

        /* TODO this can be optimized:
         * we actually do not need newStrokes as temporary storage */

        layer.items[i] = newStrokes[0];
        for (int j = 1; j < newStrokes.length(); j ++) {
            layer.items.insert(i + j, newStrokes[j]);
        }
        i += newStrokes.length() - 1;
        newStrokes.clear();
        update();
    }
}
