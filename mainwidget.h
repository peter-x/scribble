#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtGui>

class MainWidget;

#include "scribblearea.h"
#include "scribble_document.h"

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = 0);
    void loadFile(const QFile&);

    ScribblePage *getPage(int num);

signals:

public slots:
    void save();

private slots:
    void usePen();
    void useEraser();


protected:
    void keyPressEvent(QKeyEvent *);

private:
    QDir data_dir;
    ScribbleArea *scribbleArea;
    ScribbleDocument *document;
};

#endif // MAINWIDGET_H
