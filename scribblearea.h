#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QWidget>

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
};

#endif // SCRIBBLEAREA_H
