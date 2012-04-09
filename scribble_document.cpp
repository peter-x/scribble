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

#include "scribble_document.h"

#include <QtXml/QtXml>
#include <QHash>
#include <QColor>

#include "fileio.h"

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

QByteArray ScribblePage::getXmlRepresentation() const
{
    /* TODO Can we cache this? During autosave, it is computed in the
     * other thread. Getting the information back would save
     * quite some CPU. */
    QByteArray output;
    output += QString().sprintf("<page width=\"%.2f\" height=\"%.2f\">\n",
                                size.width(),
                                size.height()).toUtf8();

    ScribbleXournalBackground back = background;
    if (back.type.isNull()) {
        /* default background style */
        back.type = "solid";
        back.style = "plain";
        back.color = "white";
    }

    output += "<background type=\"" + XournalXMLHandler::encodeString(back.type).toUtf8() + "\" ";
    if (!back.color.isNull())
        output += "color=\"" + XournalXMLHandler::encodeString(back.color).toUtf8() + "\" ";
    if (!back.style.isNull())
        output += "style=\"" + XournalXMLHandler::encodeString(back.style).toUtf8() + "\" ";
    if (!back.domain.isNull())
        output += "domain=\"" + XournalXMLHandler::encodeString(back.domain).toUtf8() + "\" ";
    if (!back.filename.isNull())
        output += "filename=\"" + XournalXMLHandler::encodeString(back.filename).toUtf8() + "\" ";
    if (!back.pageno.isNull())
        output += "pageno=\"" + XournalXMLHandler::encodeString(back.pageno).toUtf8() + "\" ";
    output += "/>\n";

    foreach (const ScribbleLayer &layer, layers) {
        output += "<layer>\n";
        foreach (const ScribbleStroke &stroke, layer.items) {
            quint32 color = stroke.getPen().color().rgba();
            /* alpha channel is msB in Qt and lsB in Xournal */
            color = (color << 8) | (color >> 24);
            output += QString().sprintf("<stroke tool=\"pen\" color=\"#%08x\" width=\"%.2f\">",
                     color, stroke.getPen().widthF()).toUtf8();
            int pos = output.size();
            output.resize(output.size() + stroke.getPoints().size() * 14);
            foreach (const QPointF &point, stroke.getPoints()) {
                int written;
                while (1) {
                    written = snprintf(output.data() + pos, output.size() - pos,
                                   "%.2f %.2f ", point.x(), point.y());
                    if (written >= output.size() - pos) {
                        output.resize(output.size() + 1024);
                    } else {
                        break;
                    }
                }
                pos += written;
            }
            output.resize(pos);
            output += "\n</stroke>\n";
            /* TODO error for text items */
        }
        output += "</layer>\n";
    }
    output += "</page>\n";
    return output;
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
    static QRegExp splitter("\\s+");

    if (localName == "stroke") {
        ScribbleStroke &s = pages.last().layers.last().items.last();
        QStringList chunks = currentStrokeString.split(splitter, QString::SkipEmptyParts);
        QVector<QPointF> points(chunks.length() / 2);
        for (int i = 0; i < chunks.length() - 1; i += 2) {
            points[i / 2] = QPointF(chunks[i].toFloat(), chunks[i + 1].toFloat());
        }
        s.appendPoints(points);
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

bool ScribbleDocument::loadXournalFile(QByteArray data)
{
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

QByteArray ScribbleDocument::toXournalXMLFormat()
{
    return toXournalXMLFormat(pages);
}

QByteArray ScribbleDocument::toXournalXMLFormat(const QList<ScribblePage> &pages)
{
    setlocale(LC_NUMERIC, "C");

    QByteArray output = "<?xml  version=\"1.0\" standalone=\"no\"?>\n"
            "<xournal version=\"0.4.5\">\n"
            "<title>Scribble document - see https://github.com/peter-x/scribble</title>\n";
    for (int i = 0; i < pages.length(); i ++) {
        output += pages[i].getXmlRepresentation();
    }
    output += "</xournal>\n";

    setlocale(LC_NUMERIC, "");

    return output;
}

void ScribbleDocument::initAfterLoad()
{
    if (pages.length() == 0) {
        pages.append(ScribblePage());
        pages[0].layers.append(ScribbleLayer());
        pages[0].invalidate();
    }
    currentPage = 0;
    currentLayer = getCurrentPage().layers.length() - 1;

    sketching = false;
    currentStroke = 0;
    currentMode = PEN;
    currentPen.setColor(QColor(0, 0, 0));
    currentPen.setWidth(1);

    changedSinceLastSave = false;

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
        p.invalidate();
        changedSinceLastSave = true;
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
        pages[currentPage].invalidate();
        changedSinceLastSave = true;
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
        pages[currentPage].invalidate();
        changedSinceLastSave = true;
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
        pages[currentPage].invalidate();
        changedSinceLastSave = true;
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

    if (!removedStrokes.isEmpty()) {
        pages[currentPage].invalidate();
        changedSinceLastSave = true;
        emit strokesChanged(getCurrentPage(), currentLayer, removedStrokes);
    }
}
