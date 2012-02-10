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

    ScribblePage *getPage(int num);
    ScribblePage *getCurrentPage();
    int getCurrentPageNumber();
    int getCurrentLayer();

signals:

public slots:
    void save();

private slots:
    void usePen();
    void useEraser();
    void previousPage();
    void nextPage();
    void sizeThin();
    void sizeMedium();
    void sizeThick();


protected:
    void keyPressEvent(QKeyEvent *);

private:
    void updateModeSizeColor();

    QDir data_dir;
    ScribbleArea *scribbleArea;
    ScribbleDocument *document;

    int currentPage;
    int currentLayer;
    ScribbleArea::ScribbleMode currentMode; /* TODO duplication */
    int currentSize; /* TODO duplication */
};

#endif // MAINWIDGET_H
