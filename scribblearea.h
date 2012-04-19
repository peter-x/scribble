/*
 * scribble: Scribbling Application for Onyx Boox M92
 *
 * Copyright (C) 2012 peter-x
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QWidget>
#include <QPainter>
#include <QTimer>

#include "scribble_document.h"

class ScribbleGraphicsContext
{
public:
    /* draw to QWidget */
    ScribbleGraphicsContext(QPainter *painter, bool undraw) : widget(0), painter(painter), undraw(undraw) {}
    /* directly draw to screen */
    ScribbleGraphicsContext(QWidget *widget, bool undraw) : widget(widget), painter(0), undraw(undraw) {}

    void drawPage(const ScribblePage &page, int maxLayer);
    void drawStroke(const ScribbleStroke &stroke);
    void drawStrokeSegment(const ScribbleStroke &stroke, int i);

private:
    void drawLinePainter(const QPoint &p1, const QPoint &p2, unsigned char color, int width);
    void drawLineDirect(const QPoint &p1, const QPoint &p2, unsigned char color, int width);

    QWidget *widget;
    QPainter *painter;
    bool undraw;
};

class ScribbleArea : public QWidget
{
    Q_OBJECT
public:

    explicit ScribbleArea(QWidget *parent, const ScribbleDocument *document);

signals:
    void resized(const QSize &size);

public slots:
    void redrawPage(const ScribblePage &page, int layer);
    void drawLastStrokeSegment(const ScribbleStroke &);
    void drawCompletedStroke(const ScribbleStroke &);

    void updateStrokes(const ScribblePage &page, int layer, const QList<ScribbleStroke> &removedStrokes);

protected:
    void resizeEvent(QResizeEvent *);

private slots:
    void updateIfNeeded();

private:
    void paintEvent(QPaintEvent *);
    /* uses painter on x86 */
    void drawStroke(const ScribbleStroke &s, bool unpaint = false);
    /* uses painter on x86 */
    void drawStrokeSegment(const ScribbleStroke &s, int i, bool unpaint = false);

    const ScribbleDocument *document;

    QImage buffer;

    QRegion regionToUpdate;
    QTimer updateTimer;
};

#endif // SCRIBBLEAREA_H
