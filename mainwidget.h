#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtGui>

#include "scribblearea.h"
#include "scribble_document.h"

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = 0);
    void loadFile(const QFile&);

signals:

public slots:
    void save();

private slots:
    void touchEventDataReceived(TouchData &);
    void mousePressEvent(QMouseEvent *ev) { document->mousePressEvent(ev); }
    void mouseMoveEvent(QMouseEvent *ev) { document->mouseMoveEvent(ev); }
    void mouseReleaseEvent(QMouseEvent *ev) { document->mouseReleaseEvent(ev); }


protected:
    void keyPressEvent(QKeyEvent *);

private:
    TouchEventListener touchListener;
    int pressure_of_last_point_;

    QDir data_dir;
    ScribbleArea *scribbleArea;
    ScribbleDocument *document;
};

#endif // MAINWIDGET_H
