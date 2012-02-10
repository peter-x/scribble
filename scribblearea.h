#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QWidget>

#include "onyx/touch/touch_listener.h"

#include "scribble_document.h"

class ScribbleArea : public QWidget
{
    Q_OBJECT
public:
    enum ScribbleMode {
        PEN, ERASER
    };

    explicit ScribbleArea(QWidget *parent);
    void setModeSizeColor(ScribbleMode mode, float size, const QColor &color);

signals:

public slots:
    void pageChanged(ScribblePage *page, int layer);

private slots:
    void touchEventDataReceived(TouchData &);

private:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);


    void eraseAt(const QPointF &point);

    ScribblePage *currentPage;
    int currentLayer;

    QPen currentPen;

    bool sketching;
    QPolygonF currentStroke;

    ScribbleMode currentMode;

    TouchEventListener touchListener;
    int pressure_of_last_point_;
};

#endif // SCRIBBLEAREA_H
