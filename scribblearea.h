#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QWidget>
#include <QPainter>

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
    void drawLastStrokeSegment(const ScribbleStroke &);
    void drawCompletedStroke(const ScribbleStroke &);

    void updateStrokes(const ScribblePage &page, int layer, const QList<ScribbleStroke> &removedStrokes);

protected:
    void resizeEvent(QResizeEvent *);

private slots:
#ifndef BUILD_FOR_ARM
    void updateIfNeeded();

#endif

private:
    void paintEvent(QPaintEvent *);
    /* uses painter on x86 */
    void drawStroke(const ScribbleStroke &s, bool unpaint = false);
    /* uses painter on x86 */
    void drawStrokeSegment(const ScribbleStroke &s, int i, bool unpaint = false);

#ifndef BUILD_FOR_ARM
    bool needUpdate;
    QImage buffer;
    QPainter painter;
    QTimer updateTimer;
#endif
};

#endif // SCRIBBLEAREA_H
