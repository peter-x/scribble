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

#include <QtXml/QXmlDefaultHandler>

class ScribbleStroke
{
public:
    /* TODO cache value (if not done by QPolygonF) */
    QRectF getBoundingBox() const {
        return points.boundingRect();
    }

    QPen pen;
    QPolygonF points;
};

class ScribbleLayer
{
public:
    QList<ScribbleStroke> items;
};

class ScribblePage
{
public:
    QList<ScribbleLayer> layers;
    QSizeF size;
    /* TODO background */
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
    QString title;
    QList<ScribblePage> pages;

    explicit ScribbleDocument(QObject *parent = 0);
    bool loadXournalFile(const QFile &file);

signals:

public slots:


};


#endif // SCRIBBLE_DOCUMENT_H
