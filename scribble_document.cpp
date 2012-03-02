#include "scribble_document.h"

#include <QtXml/QtXml>
#include <QHash>
#include <QColor>

#include "zlib.h"

bool ScribbleStroke::segmentIntersects(int i, const ScribbleStroke &o) const
{
    if (i < 0 || i + 1 >= points.size()) return false;

    QPointF p1(points[i]);
    QPointF p2(points[i + 1]);

    /* TODO
     * first check if the bounding box of this segment (plus width!) intersects the stroke
     * then check for each segment of the stroke if they intersect */
    return true;
}

void ScribbleStroke::updateBoundingRect()
{
    boundingRect = points.boundingRect();
    qreal a = pen.widthF() / 2.0;
    a = qMin(a, qreal(0.6));
    /* bounding rect with zero width or height produces not the intended result */
    boundingRect.adjust(-a, -a, a, a);
}

/* ---------------------------------------------------------------- */

XournalXMLHandler::XournalXMLHandler()
{
    xournal_colors["black"] = QColor("#000000");
    xournal_colors["blue"] = QColor("#3333cc");
    xournal_colors["red"] = QColor("#ff0000");
    xournal_colors["green"] = QColor("#008000");
    xournal_colors["gray"] = QColor("#808080");
    xournal_colors["lightblue"] = QColor("#00c00f");
    xournal_colors["lightgreen"] = QColor("#00ff00");
    xournal_colors["magenta"] = QColor("#ff00ff");
    xournal_colors["orange"] = QColor("#ff8000");
    xournal_colors["yellow"] = QColor("#ffff00");
    xournal_colors["white"] = QColor("#ffffff");
}

bool XournalXMLHandler::startDocument()
{
    title.clear();
    pages.clear();
    currentLocalName.clear();
    return true;
}

bool XournalXMLHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(qName);

    if (localName == "stroke") {
        /* TODO tools that are not pen */
        //if (atts.value("tool") != "pen") return false;
        /* TODO error for tools that are not pen */

        QPen pen;
        ScribbleStroke stroke;
        pen.setColor(xournal_colors[atts.value("color")]);
        pen.setWidthF(atts.value("width").toFloat());
        stroke.setPen(pen);
        pages.last().layers.last().items.append(stroke);
    } else if (localName == "page") {
        ScribblePage p;
        p.size = QSizeF(atts.value("width").toFloat(), atts.value("height").toFloat());
        pages.append(p);
    } else if (localName == "background") {
        ScribblePage &p(pages.last());
        p.background.type = getAttribute(atts, "type");
        p.background.color = getAttribute(atts, "color");
        p.background.style = getAttribute(atts, "style");
        p.background.domain = getAttribute(atts, "domain");
        p.background.filename = getAttribute(atts, "filename");
        p.background.pageno = getAttribute(atts, "pageno");
    } else if (localName == "layer") {
        ScribbleLayer l;
        pages.last().layers.append(l);
    } else  if (localName == "title") {
            title.clear();
    } else if (localName == "xournal") {
    } else {
        return false;
    }
    currentLocalName = localName;
    return true;
}

bool XournalXMLHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(qName);

    if (localName == "stroke") {
        ScribbleStroke &s = pages.last().layers.last().items.last();
        QStringList chunks = currentStrokeString.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        for (int i = 0; i < chunks.length() - 1; i += 2) {
            /* TODO appendPoints? */
            s.appendPoint(QPointF(chunks[i].toFloat(), chunks[i + 1].toFloat()));
        }
        currentStrokeString.clear();
    }
    currentLocalName.clear();
    return true;
}

bool XournalXMLHandler::characters(const QString &ch)
{
    if (currentLocalName == "stroke") {
        currentStrokeString += ch;
    } else if (currentLocalName == "title") {
        title += ch;
    }
    return true;
}

bool XournalXMLHandler::ignorableWhitespace(const QString &ch)
{
    if (currentLocalName == "stroke") {
        currentStrokeString += ch;
    } else if (currentLocalName == "title") {
        title += ch;
    }
    return true;
}

bool XournalXMLHandler::endDocument()
{
    if (pages.isEmpty()) {
        pages.append(ScribblePage());
    }
    for (int i = 0; i < pages.length(); i ++) {
        if (pages[i].layers.isEmpty()) {
            pages[i].layers.append(ScribbleLayer());
        }
    }

    return true;
}

bool XournalXMLHandler::fatalError(const QXmlParseException & exception)
{
    qWarning() << "Fatal error on line" << exception.lineNumber()
               << ", column" << exception.columnNumber() << ":"
               << exception.message();

    return false;
}

/* --------------------------------------------------------------- */

bool EraserContext::erase(const ScribbleStroke *stroke, QList<ScribbleStroke> *removedStrokes, QList<ScribbleStroke> *newStrokes,
                          const QPointF &point, qreal width)
{
    this->stroke = stroke;
    this->removedStrokes = removedStrokes;
    this->newStrokes = newStrokes;

    previousPointRemoved = false;
    previousChangeIndex = 0;

    qreal halfWidthSq = width * width / 4.0;

    /* we only check intersections with points, not
     * line segments */
    for (int i = 0; i < stroke->getPoints().size(); i ++) {
        QPointF p = stroke->getPoints()[i] - point;
        qreal d = p.x() * p.x() + p.y() * p.y();
        if (d <= halfWidthSq) {
            /* remove point */
            if (!previousPointRemoved) {
                appendFromPreviousChangeIndexUpTo(i, newStrokes);
            }
            previousPointRemoved = true;
        } else {
            /* retain point */
            if (previousPointRemoved) {
                appendFromPreviousChangeIndexUpTo(i, removedStrokes);
            }
            previousPointRemoved = false;
        }
    }
    eraseEnded();
    return previousPointRemoved || previousChangeIndex > 0;
}

void EraserContext::eraseEnded()
{
    int endIndex = stroke->getPoints().size() - 1;
    if (previousPointRemoved) {
        appendFromPreviousChangeIndexUpTo(endIndex, removedStrokes);
    } else {
        if (previousChangeIndex == 0) {
            /* nothing happened */
            return;
        } else {
            appendFromPreviousChangeIndexUpTo(endIndex, newStrokes);
        }
    }
}

void EraserContext::appendFromPreviousChangeIndexUpTo(int endIndex, QList<ScribbleStroke> *list)
{
    if (endIndex > previousChangeIndex) {
        list->append(ScribbleStroke(stroke->getPen(),
                                    stroke->getPoints().mid(previousChangeIndex,
                                                            endIndex - previousChangeIndex + 1)));
    }
    previousChangeIndex = endIndex;
}

/* --------------------------------------------------------------- */

ScribbleDocument::ScribbleDocument(QObject *parent) :
    QObject(parent), title("")
{
    initAfterLoad();
}

bool ScribbleDocument::loadXournalFile(const QFile &file)
{
    /* TODO do we have to free anything during file name conversion? */
    gzFile f = gzopen(file.fileName().toLocal8Bit().constData(), "r");
    if (f == 0) {
        return false;
    }

    QByteArray data;

    {
        char buffer[1024];
        while (!gzeof(f)) {
            int len = gzread(f, buffer, 1024);
            if (len < 0) {
                gzclose(f);
                return false;
            }
            data.append(buffer, len);
        }
        gzclose(f);
    }

    QBuffer dataBuffer(&data);
    QXmlInputSource source(&dataBuffer);
    QXmlSimpleReader reader;
    XournalXMLHandler handler;
    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);
    if (!reader.parse(&source, false)) {
        return false;
    }

    title = handler.getTitle();
    pages = handler.getPages();
    initAfterLoad();

    /* TODO only save to this file again if we were able
     * to read all content correctly */

    emit pageOrLayerNumberChanged(currentPage, pages.length(), currentLayer, getCurrentPage().layers.length());
    emit pageOrLayerChanged(getCurrentPage(), currentLayer);
    return true;
}

bool ScribbleDocument::saveXournalFile(const QFile &file)
{
    /* TODO do we have to free anything during file name conversion? */
    gzFile f = gzopen(file.fileName().toLocal8Bit().constData(), "w");
    if (f == 0) {
        return false;
    }

    setlocale(LC_NUMERIC, "C");

    gzprintf(f, "<?xml  version=\"1.0\" standalone=\"no\"?>\n"
            "<xournal version=\"0.4.5\">\n"
            "<title>Scribble document - see https://github.com/peter-x/scribble</title>\n");
    foreach (const ScribblePage &page, pages) {
        gzprintf(f, "<page width=\"%.2f\" height=\"%.2f\">\n",
                 page.size.width(),
                 page.size.height());

        ScribbleXournalBackground background = page.background;
        if (background.type.isNull()) {
            /* default background style */
            background.type = "solid";
            background.style = "plain";
            background.color = "white";
        }

        gzprintf(f, "<background type=\"%s\" ", XournalXMLHandler::encodeString(background.type).toUtf8().constData());
        if (!background.color.isNull())
            gzprintf(f, "color=\"%s\" ", XournalXMLHandler::encodeString(background.color).toUtf8().constData());
        if (!background.style.isNull())
            gzprintf(f, "style=\"%s\" ", XournalXMLHandler::encodeString(background.style).toUtf8().constData());
        if (!background.domain.isNull())
            gzprintf(f, "domain=\"%s\" ", XournalXMLHandler::encodeString(background.domain).toUtf8().constData());
        if (!background.filename.isNull())
            gzprintf(f, "filename=\"%s\" ", XournalXMLHandler::encodeString(background.filename).toUtf8().constData());
        if (!background.pageno.isNull())
            gzprintf(f, "pageno=\"%s\" ", XournalXMLHandler::encodeString(background.pageno).toUtf8().constData());
        gzprintf(f, "/>\n");

        foreach (const ScribbleLayer &layer, page.layers) {
            gzprintf(f, "<layer>\n");
            foreach (const ScribbleStroke &stroke, layer.items) {
                quint32 color = stroke.getPen().color().rgba();
                /* alpha channel is msB in Qt and lsB in Xournal */
                color = (color << 8) | (color >> 24);
                gzprintf(f, "<stroke tool=\"pen\" color=\"#%08x\" width=\"%.2f\">",
                         color, stroke.getPen().widthF());
                foreach (const QPointF &point, stroke.getPoints()) {
                    gzprintf(f, "%.2f %.2f ", point.x(), point.y());
                }
                gzprintf(f, "\n</stroke>\n");
                /* TODO error for text items */
            }
            gzprintf(f, "</layer>\n");
        }
        gzprintf(f, "</page>\n");
    }
    gzprintf(f, "</xournal>\n");
    gzclose(f);

    setlocale(LC_NUMERIC, "");

    return true;
 }

void ScribbleDocument::initAfterLoad()
{
    if (pages.length() == 0) {
        pages.append(ScribblePage());
        pages[0].layers.append(ScribbleLayer());
    }
    currentPage = 0;
    currentLayer = getCurrentPage().layers.length() - 1;

    sketching = false;
    currentStroke = 0;
    currentMode = PEN;
    currentPen.setColor(QColor(0, 0, 0));
    currentPen.setWidth(1);

    emit pageOrLayerNumberChanged(currentPage, pages.length(), currentLayer, getCurrentPage().layers.length());
    emit pageOrLayerChanged(getCurrentPage(), currentLayer);
}

void ScribbleDocument::endCurrentStroke()
{
    if (!sketching) return;

    if (currentMode == PEN) {
        emit strokeCompleted(*currentStroke);
        currentStroke = 0;
    }
    sketching = false;
}

bool ScribbleDocument::setCurrentPage(int index)
{
    if (index < 0 || index >= pages.length())
        return false;
    endCurrentStroke();
    if (currentLayer == getCurrentPage().layers.length() - 1) {
        currentPage = index;
        currentLayer = getCurrentPage().layers.length() - 1;
    } else {
        currentPage = index;
        currentLayer = qMin(currentLayer, getCurrentPage().layers.length() - 1);
    }
    emit pageOrLayerNumberChanged(currentPage, pages.length(), currentLayer, getCurrentPage().layers.length());
    emit pageOrLayerChanged(getCurrentPage(), currentLayer);
    return true;
}

void ScribbleDocument::nextPage()
{
    if (currentPage + 1 >= pages.length()) {
        ScribblePage p;
        if (!currentViewSize.isEmpty())
            p.size = currentViewSize;
        p.layers.append(ScribbleLayer());
        pages.append(p);
    }
    setCurrentPage(currentPage + 1);
}

void ScribbleDocument::previousPage()
{
    setCurrentPage(currentPage - 1);
}

void ScribbleDocument::layerUp()
{
    endCurrentStroke();
    ScribblePage &p = pages[currentPage];
    if (currentLayer + 1 >= p.layers.length()) {
        p.layers.append(ScribbleLayer());
    }
    currentLayer += 1;
    emit pageOrLayerNumberChanged(currentPage, pages.length(), currentLayer, getCurrentPage().layers.length());
    emit pageOrLayerChanged(getCurrentPage(), currentLayer);
}

void ScribbleDocument::layerDown()
{
    if (currentLayer == 0) return;
    endCurrentStroke();
    currentLayer -= 1;
    emit pageOrLayerNumberChanged(currentPage, pages.length(), currentLayer, getCurrentPage().layers.length());
    emit pageOrLayerChanged(getCurrentPage(), currentLayer);
}

void ScribbleDocument::mousePressEvent(QMouseEvent *event)
{
    if (sketching) /* should not happen */
        endCurrentStroke();

    sketching = true;
    if (currentMode == PEN) {
        ScribbleLayer &l = pages[currentPage].layers[currentLayer];
        l.items.append(ScribbleStroke(currentPen, QPolygonF()));
        currentStroke = &l.items.last();
        currentStroke->appendPoint(event->posF());
        emit strokePointAdded(*currentStroke);
    } else {
        eraseAt(event->pos());
    }
}

void ScribbleDocument::mouseMoveEvent(QMouseEvent *event)
{
    if (!sketching) return;
    if (currentMode == PEN) {
        currentStroke->appendPoint(event->posF());
        emit strokePointAdded(*currentStroke);
    } else {
        eraseAt(event->pos());
    }
}

void ScribbleDocument::mouseReleaseEvent(QMouseEvent *event)
{
    if (!sketching) return;

    if (currentMode == PEN) {
        currentStroke->appendPoint(event->posF());
        emit strokePointAdded(*currentStroke);
    } else {
        eraseAt(event->pos());
    }
    endCurrentStroke();
}

void ScribbleDocument::eraseAt(const QPointF &point)
{
    ScribbleLayer &layer = pages[currentPage].layers[currentLayer];

    qreal width = currentPen.widthF();
    QRectF eraserBox;
    eraserBox.setSize(QSizeF(width, width));
    eraserBox.moveCenter(point);

    QList<ScribbleStroke> removedStrokes;
    QList<ScribbleStroke> newStrokes;

    EraserContext eraserContext;

    for (int i = 0; i < layer.items.length(); i ++) {
        ScribbleStroke &s = layer.items[i];
        if (!s.boundingRectIntersects(eraserBox))
            continue;

        if (!eraserContext.erase(&s, &removedStrokes, &newStrokes, point, width)) {
            /* nothing removed */
            continue;
        }

        if (newStrokes.isEmpty()) {
            /* stroke removed completely */
            layer.items.removeAt(i);
            i -= 1;
        } else {
            layer.items[i] = newStrokes[0];
            for (int j = 1; j < newStrokes.length(); j ++) {
                layer.items.insert(i + j, newStrokes[j]);
            }
            i += newStrokes.length() - 1;
            newStrokes.clear();
        }
    }

    if (!removedStrokes.isEmpty())
        emit strokesChanged(getCurrentPage(), currentLayer, removedStrokes);
}
