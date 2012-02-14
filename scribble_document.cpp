#include "scribble_document.h"

#include <QtXml/QtXml>
#include <QHash>
#include <QColor>

#include "zlib.h"

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

        ScribbleStroke stroke;
        stroke.pen.setColor(xournal_colors[atts.value("color")]);
        stroke.pen.setWidthF(atts.value("width").toFloat());
        pages.last().layers.last().items.append(stroke);
    } else if (localName == "page") {
        ScribblePage p;
        p.size = QSizeF(atts.value("width").toFloat(), atts.value("height").toFloat());
        pages.append(p);
    } else if (localName == "background") {
        /* TODO */
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
        QPolygonF &poly = pages.last().layers.last().items.last().points;
        QStringList chunks = currentStrokeString.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        for (int i = 0; i < chunks.length() - 1; i += 2) {
            poly.append(QPointF(chunks[i].toFloat(), chunks[i + 1].toFloat()));
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

    emit pageOrLayerChanged(getCurrentPage(), currentLayer);
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
    currentMode = PEN;
    currentPen.setColor(QColor(0, 0, 0));
    currentPen.setWidth(1);
}

bool ScribbleDocument::setCurrentPage(int index)
{
    if (index < 0 || index >= pages.length())
        return false;
    currentPage = index;
    currentLayer = qMin(currentLayer, getCurrentPage().layers.length() - 1);
    emit pageOrLayerChanged(getCurrentPage(), currentLayer);
    return true;
}

void ScribbleDocument::nextPage()
{
    if (currentPage + 1 >= pages.length()) {
        ScribblePage p;
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
    ScribblePage &p = pages[currentPage];
    if (currentLayer + 1 >= p.layers.length()) {
        p.layers.append(ScribbleLayer());
    }
    currentLayer += 1;
    emit pageOrLayerChanged(getCurrentPage(), currentLayer);
}

void ScribbleDocument::layerDown()
{
    if (currentLayer == 0) return;
    currentLayer -= 1;
    emit pageOrLayerChanged(getCurrentPage(), currentLayer);
}

void ScribbleDocument::mousePressEvent(QMouseEvent *event)
{
    /* TODO currentStroke needs to be part of the page, otherwise
     * complete repaint event handles will not paint it */
    /* TODO what to do if not empty? */
    currentStroke.points.clear();
    currentStroke.pen = currentPen;
    sketching = true;
    if (currentMode == PEN) {
        currentStroke.points.append(event->pos());
        emit strokePointAdded(currentStroke);
    } else {
        eraseAt(event->pos());
    }
}

void ScribbleDocument::mouseMoveEvent(QMouseEvent *event)
{
    if (!sketching) return;
    if (currentMode == PEN) {
        currentStroke.points.append(event->pos());
        emit strokePointAdded(currentStroke);
    } else {
        eraseAt(event->pos());
    }
}

void ScribbleDocument::mouseReleaseEvent(QMouseEvent *event)
{
    if (!sketching) return;

    if (currentMode == PEN) {
        currentStroke.points.append(event->pos());
        emit strokePointAdded(currentStroke);

        /* TODO do we need to copy? */
        pages[currentPage].layers[currentLayer].items.append(currentStroke);
        emit strokeCompleted(currentStroke);
        currentStroke.points.clear();
    } else {
        eraseAt(event->pos());
    }

    sketching = false;
}

void ScribbleDocument::eraseAt(const QPointF &point)
{
    ScribbleLayer &layer = pages[currentPage].layers[currentLayer];

    float width = currentPen.widthF();
    float halfWidthSq = width * width / 4.0;
    QRectF eraserBox;
    eraserBox.setSize(QSizeF(width, width));
    eraserBox.moveCenter(point);

    bool somethingHappened = false;

    QList<ScribbleStroke> newStrokes;
    for (int i = 0; i < layer.items.length(); i ++) {
        ScribbleStroke &s = layer.items[i];
        /* bounding rect with zero width or height produces not the intended result */
        QRectF bound = s.points.boundingRect();
        bound.adjust(-1, -1, 1, 1);
        if (!bound.intersects(eraserBox))
            continue;

        /* we only check intersections with points, not
         * line segments */
        int retainSequenceStartIndex = 0;
        for (int j = 0; j < s.points.size(); j ++) {
            QPointF p = s.points[j] - point;
            float d = p.x() * p.x() + p.y() * p.y();
            if (d < halfWidthSq) {
                /* remove this point */
                if (retainSequenceStartIndex >= 0) {
                    /* did not remove last point */
                    ScribbleStroke newStroke;
                    newStroke.pen = s.pen;
                    newStroke.points = s.points.mid(retainSequenceStartIndex,
                                                    j - retainSequenceStartIndex);
                    newStrokes.append(newStroke);
                    retainSequenceStartIndex = -1;
                } else {
                    /* also removed last point, do nothing */
                }
            } else {
                /* retain this point */
                if (retainSequenceStartIndex < 0) {
                    /* removed last point */
                    retainSequenceStartIndex = j;
                }
            }
        }
        if (retainSequenceStartIndex == 0) {
            /* no point removed */
            continue;
        }
        if (retainSequenceStartIndex > 0) {
            /* removed some point and have to add remaining points */
            ScribbleStroke newStroke;
            newStroke.pen = s.pen;
            newStroke.points = s.points.mid(retainSequenceStartIndex);
            newStrokes.append(newStroke);
        }

        /* TODO this can be optimized:
         * we actually do not need newStrokes as temporary storage */

        layer.items[i] = newStrokes[0];
        for (int j = 1; j < newStrokes.length(); j ++) {
            layer.items.insert(i + j, newStrokes[j]);
        }
        i += newStrokes.length() - 1;
        newStrokes.clear();

        somethingHappened = true;
    }

    if (somethingHappened)
        /* TODO larger eraser box? */
        emit strokesChanged(getCurrentPage(), currentLayer, eraserBox);
}
