#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QWidget>

class ScribbleArea;

#include "mainwidget.h"
#include "scribble_document.h"



class ScribbleArea : public QWidget
{
    Q_OBJECT
public:
    enum ScribbleMode {
        PEN, ERASER
    };

    explicit ScribbleArea(MainWidget *parent);
    void setPageLayer(int page, int layer);
    void setModeSizeColor(ScribbleMode mode, float size, const QColor &color);

signals:

public slots:

private:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void eraseAt(const QPointF &point);

    MainWidget *mainWidget;

    QPen currentPen;

    bool sketching;
    QPolygonF currentStroke;

    ScribbleMode currentMode;
    int currentPage;
    int currentLayer;
};

#endif // SCRIBBLEAREA_H
