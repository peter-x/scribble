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

#ifndef SCRIBBLE_DOCUMENT_H
#define SCRIBBLE_DOCUMENT_H

#include <QObject>
#include <QSizeF>
#include <QPoint>
#include <QColor>
#include <QHash>
#include <QPen>
#include <QPolygonF>
#include <QFile>
#include <QMouseEvent>

#include <QtXml/QXmlDefaultHandler>

class ScribbleStroke
{
public:
    ScribbleStroke() {}
    ScribbleStroke(const QPen &pen, const QPolygonF &points) : pen(pen), points(points) { updateBoundingRect(); }
    const QPolygonF &getPoints() const { return points; }
    const QPen &getPen() const { return pen; }
    void setPen(const QPen &pen) { this->pen = pen; updateBoundingRect(); }

    const QRectF &getBoundingRect() const { return boundingRect; }
    bool segmentIntersects(int i, const ScribbleStroke &o) const;
    bool boundingRectIntersects(const ScribbleStroke &o) const { return boundingRectIntersects(o.boundingRect); }
    bool boundingRectIntersects(const QRectF &r) const { return boundingRect.intersects(r); }
    void appendPoint(const QPointF &p) { points.append(p); updateBoundingRect(); }
    void appendPoints(const QVector<QPointF> &p) { points += p; updateBoundingRect(); }

private:
    void updateBoundingRect();

    QPen pen;
    QPolygonF points;

    QRectF boundingRect;
};

class ScribbleLayer
{
public:
    QList<ScribbleStroke> items;
};

class ScribbleXournalBackground
{
public:
    /* if these are null (QString::isNull), the
     * respective attributes are not present */
    QString type;
    QString color;
    QString style;
    QString domain;
    QString filename;
    QString pageno;
};

class ScribblePage
{
public:
    ScribblePage() : size(QSizeF(612, 792)) {} /* TODO use reasonable values */
    QList<ScribbleLayer> layers;
    QSizeF size;
    ScribbleXournalBackground background;

    void invalidate() const {
        /* TODO could be used to invalide cached XML representation */
    }
    QByteArray getXmlRepresentation() const;
};

class EraserContext
{
public:
    EraserContext() {}
    bool erase(const ScribbleStroke *stroke, QList<ScribbleStroke> *removedStrokes, QList<ScribbleStroke> *newStrokes,
               const QPointF &point, qreal width);

private:
    void appendFromPreviousChangeIndexUpTo(int endIndex, QList<ScribbleStroke> *list);
    void eraseEnded();

    bool previousPointRemoved;
    int previousChangeIndex;

    const ScribbleStroke *stroke;
    QList<ScribbleStroke> *removedStrokes;
    QList<ScribbleStroke> *newStrokes;
};

class XournalXMLHandler : public QXmlDefaultHandler {
public:
    XournalXMLHandler();
    bool startDocument();
    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
    bool characters(const QString &ch);
    bool ignorableWhitespace(const QString &ch);
    bool endDocument();

    bool fatalError(const QXmlParseException &exception);

    /* convenience functions for reading and writing */
    static QString encodeString(const QString &str) {
        QString text = str;
        return text.replace("&", "&amp;").replace("\"","&quot;")
                .replace("'", "&apos;").replace("<", "&lt;")
                .replace(">", "&gt;");
    }
    /* returns the null string if the attribute does not exist */
    static QString getAttribute(const QXmlAttributes &attrs, const QString &attr) {
        return attrs.index(attr) < 0 ? QString() : attrs.value(attr);
    }

public:
    QString getTitle() const { return title; }
    QList<ScribblePage> getPages() const { return pages; }

private:
    QString title;
    QList<ScribblePage> pages;

    QString currentLocalName;
    QString currentStrokeString;

    QHash<QString, QColor> xournal_colors;
};

class ScribbleDocument : public QObject
{
    Q_OBJECT
public:
    explicit ScribbleDocument(QObject *parent = 0);
    bool loadXournalFile(QByteArray data);
    QByteArray toXournalXMLFormat();
    static QByteArray toXournalXMLFormat(const QList<ScribblePage> &pages);
    /* TODO this will cause deep copies to occur upon the first
     * change (i.e. first mouse move) */
    QList<ScribblePage> getPagesCopy() const { return pages; }

    int getNumPages() const { return pages.length(); }
    const ScribblePage &getCurrentPage() const { return pages[currentPage]; }
    int getCurrentLayer() const { return currentLayer; }
    bool hasChangedSinceLastSave() const { return changedSinceLastSave; }
    void setSaved() { changedSinceLastSave = false; }

signals:
    void pageOrLayerNumberChanged(int currentPage, int maxPages, int currentLayer, int maxLayers);
    /* only if changed completely */
    void pageOrLayerChanged(const ScribblePage &page, int currentLayer);
    /* Only the last point was added. The stroke is the newest one. */
    void strokePointAdded(const ScribbleStroke &);
    /* Is emitted after all points have been added. */
    void strokeCompleted(const ScribbleStroke &);

    void strokesChanged(const ScribblePage &page, int layer, const QList<ScribbleStroke> &removedStrokes);

public slots:
    void usePen() { endCurrentStroke(); currentMode = PEN; currentPen.setWidth(2); }
    void useEraser() { endCurrentStroke(); currentMode = ERASER; currentPen.setWidth(10); }

    bool setCurrentPage(int index);

    /* will create new page if at end of document */
    void nextPage();
    void previousPage();

    /* will create a new layer if at end of number of layers */
    void layerUp();
    void layerDown();

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void setViewSize(const QSize &size) { currentViewSize = size; }
private:
    enum ScribbleMode {
        PEN, ERASER
    };

    void initAfterLoad();
    void endCurrentStroke();
    void eraseAt(const QPointF &point);

    QString title;
    QList<ScribblePage> pages;

    int currentPage;
    int currentLayer;

    QPen currentPen;

    /* used when a new page is created */
    QSize currentViewSize;

    /* if currentMode == PEN && sketching then the last item in the current page
     * and layer (exists and) is currently extended by mouse movements */
    bool sketching;
    ScribbleMode currentMode;

    /* shortcut to last item of current layer (if it exists) */
    ScribbleStroke *currentStroke;

    bool changedSinceLastSave;
};


#endif // SCRIBBLE_DOCUMENT_H
