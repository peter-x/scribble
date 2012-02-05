#include "scribble_document.h"

#include <QtXml/QtXml>
#include <QHash>
#include <QColor>

#include "zlib.h"

ScribbleDocument::ScribbleDocument(QObject *parent) :
    QObject(parent)
{
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
    return true;
}

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
    return true;
}

bool XournalXMLHandler::fatalError(const QXmlParseException & exception)
{
    qWarning() << "Fatal error on line" << exception.lineNumber()
               << ", column" << exception.columnNumber() << ":"
               << exception.message();

    return false;
}
