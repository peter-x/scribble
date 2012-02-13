#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QWidget>

#include "onyx/touch/touch_listener.h"

#include "scribble_document.h"

class ScribbleArea : public QWidget
{
    Q_OBJECT
public:

    explicit ScribbleArea(QWidget *parent);

signals:

public slots:
    void redrawPage(const ScribblePage &page, int layer);
    void drawStrokePoint(const ScribbleStroke &);
    void drawCompletedStroke(const ScribbleStroke &);

    void updateStrokesInRegion(const ScribblePage &page, int layer, QRectF boundingBox);


private:
    void paintEvent(QPaintEvent *);
};

#endif // SCRIBBLEAREA_H
