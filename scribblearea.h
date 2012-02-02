#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QWidget>

#include "onyx/data/sketch_proxy.h"


class ScribbleArea : public QWidget
{
    Q_OBJECT
public:
    explicit ScribbleArea(QWidget *parent = 0);

signals:

public slots:

private:
    void paintEvent(QPaintEvent *);
    bool event(QEvent *);
    void updateSketchProxy();

    sketch::SketchProxy *sketchProxy;

};

#endif // SCRIBBLEAREA_H
