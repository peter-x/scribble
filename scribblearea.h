#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QWidget>

#include "onyx/data/sketch_proxy.h"

#include "scribble_document.h"


class ScribbleArea : public QWidget
{
    Q_OBJECT
public:
    explicit ScribbleArea(QWidget *parent = 0);
    void setModeShapeColor(const ui::SketchMode, const ui::SketchShape,
                           const ui::SketchColor);

    /* TODO preliminary method to check if loading works,
      should be expanded to MVC pattern */
    void renderPage(ScribblePage *page);

signals:

public slots:

private:
    void paintEvent(QPaintEvent *);
    bool event(QEvent *);
    void updateSketchProxy();

    sketch::SketchProxy *sketchProxy;

    ScribblePage *currentPage;

};

#endif // SCRIBBLEAREA_H
