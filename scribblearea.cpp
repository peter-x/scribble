#include "scribblearea.h"

#include <QPainter>
#include <QPen>
#include <QMouseEvent>

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"

static const QString SCRIBBLE_PATH = "scribble_doc";

ScribbleArea::ScribbleArea(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint),
    currentPage(0),
    sketching(false)
{
    setMinimumSize(100, 100);
    setAutoFillBackground(false); /* TODO change */
    setBackgroundRole(QPalette::Base);

    onyx::screen::watcher().addWatcher(this);
    connect(&touchListener, SIGNAL(touchData(TouchData &)), this, SLOT(touchEventDataReceived(TouchData &)));
}

void ScribbleArea::pageChanged(ScribblePage *page, int layer)
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
    return;
    /* TODO only paint event->rect() */
    QPainter painter(this);

    if (currentPage == 0) return;

    painter.setRenderHint(QPainter::Antialiasing);
    /* TODO Xournal only paints up to current layer */
    for (int li = 0; li < currentPage->layers.length(); li ++) {
        const ScribbleLayer &l = currentPage->layers[li];
        foreach (const ScribbleStroke &s, l.items) {
            painter.setPen(s.pen);
            //painter.drawPolyline(s.points);
            /* TODO update currently does not work on eink */
#if 0
            for (int pi = 0; pi + 1 < s.points.size(); pi ++) {
                qDebug() << "Drawing line: " << s.points[pi].x() << "," << s.points[pi].y() << "," <<
                            s.points[pi + 1].x() << "," << s.points[pi + 1].y() << "," <<
                            0x00 /* color */ << "," <<
                        1 /* size */;
                onyx::screen::instance().drawLine(s.points[pi].x(), s.points[pi].y(),
                                                  s.points[pi + 1].x(), s.points[pi + 1].y(),
                                                  0x00 /* color */,
                                                  1 /* size */);
            }
#endif
            /* TODO it seems that drawLine is broken and drawLines works (at least for a single line) */
            {
                QPolygon strokeInt(s.points.toPolygon());

                onyx::screen::instance().drawLines(strokeInt.data(), strokeInt.size(), 0x00, 2);
            }
        }
        if (li == currentLayer && sketching && currentMode == PEN) {
            painter.setPen(currentPen);
            painter.drawPolyline(currentStroke);
#if 0
            for (int pi = 0; pi + 1 < currentStroke.size(); pi ++) {
                qDebug() << "Drawing line: " << currentStroke[pi].x() << "," << currentStroke[pi].y() << "," <<
                            currentStroke[pi + 1].x() << "," << currentStroke[pi + 1].y() << "," <<
                            0x00 /* color */ << "," <<
                        1 /* size */;
                onyx::screen::instance().drawLine(currentStroke[pi].x(), currentStroke[pi].y(),
                                                  currentStroke[pi + 1].x(), currentStroke[pi + 1].y(),
                                                  0x00 /* color */,
                                                  1 /* size */);
            }
#endif
            /* TODO it seems that drawLine is broken and drawLines works (at least for a single line) */
            {
                QPolygon currentStrokeInt(currentStroke.toPolygon());

                onyx::screen::instance().drawLines(currentStrokeInt.data(), currentStrokeInt.size(), 0x00, 2);
            }
        }
    }
    //onyx::screen::instance().updateScreen(onyx::screen::ScreenProxy::DW, onyx::screen::ScreenCommand::WAIT_ALL);
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
        //QPolygon currentStrokeInt(currentStroke.toPolygon());

        if (currentStroke.size() >= 2) {
            int s = currentStroke.size();
            QPoint p1(mapToGlobal(currentStroke[s - 1].toPoint()));
            QPoint p2(mapToGlobal(currentStroke[s - 2].toPoint()));
            QVector<QPoint> line;
            line.push_back(p1);
            line.push_back(p2);
            onyx::screen::instance().drawLines(line.data(), 2, 0x00, 1);
        }
//        onyx::screen::instance().drawLines(currentStrokeInt.data(), currentStrokeInt.size(), 0x00, 2);
//        update();
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
        //update();

        ScribbleStroke stroke;
        stroke.pen = currentPen;
        stroke.points = currentStroke;

        currentStroke.clear();

        if (currentPage == 0) return;

        if (currentLayer >= 0)
            /* TODO produce some error earlier */
            currentPage->layers[currentLayer].items.append(stroke);
    }

    sketching = false;
}

void ScribbleArea::touchEventDataReceived(TouchData &data)
{
    // get widget pos
    OnyxTouchPoint &touch_point = data.points[0];
    QPoint global_pos(touch_point.x, touch_point.y);
    QPoint widget_pos = mapFromGlobal(global_pos);

    // check whether the point is in widget
    if (widget_pos.x() < 0 || widget_pos.y() < 0 ||
        widget_pos.x() > width() || widget_pos.y() > height())
    {
        // qDebug("Out of boundary");
        return;
    }

    // construct a mouse event
    QEvent::Type type = QEvent::MouseMove;
    if (pressure_of_last_point_ == 0 && touch_point.pressure > 0)
        type = QEvent::MouseButtonPress;
    if (pressure_of_last_point_ > 0 && touch_point.pressure <= 0)
        type = QEvent::MouseButtonRelease;
    /* TODO can we adjust the size depending on pressure? */

    QMouseEvent me(type, widget_pos, global_pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    switch (type)
    {
    case QEvent::MouseButtonPress:
        mousePressEvent(&me);
        break;
    case QEvent::MouseMove:
        mouseMoveEvent(&me);
        break;
    case QEvent::MouseButtonRelease:
        mouseReleaseEvent(&me);
        break;
    default:
        break;
    }
    pressure_of_last_point_ = touch_point.pressure;
}

void ScribbleArea::eraseAt(const QPointF &point)
{
    if (currentPage == 0) return;

    /* TODO error if layer does not exist */
    if (currentLayer < 0)
        return;
    ScribbleLayer &layer = currentPage->layers[currentLayer];

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
