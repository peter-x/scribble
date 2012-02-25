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

class ScribblePage
{
public:
    ScribblePage() : size(QSizeF(612, 792)) {} /* TODO use reasonable values */
    QList<ScribbleLayer> layers;
    QSizeF size;
    /* TODO background */
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
    bool loadXournalFile(const QFile &file);
    bool saveXournalFile(const QFile &file);

    int getNumPages() const { return pages.length(); }
    const ScribblePage &getCurrentPage() const { return pages[currentPage]; }
    int getCurrentLayer() const { return currentLayer; }

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

    /* if currentMode == PEN && sketching then the last item in the current page
     * and layer (exists and) is currently extended my mouse movements */
    bool sketching;
    ScribbleMode currentMode;

    /* shortcut to last item of current layer (if it exists) */
    ScribbleStroke *currentStroke;
};


#endif // SCRIBBLE_DOCUMENT_H
