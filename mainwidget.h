#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtGui>

#include "scribblearea.h"

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = 0);

signals:

public slots:

protected:
    void keyPressEvent(QKeyEvent *);

private:
    ScribbleArea *scribbleArea;
};

#endif // MAINWIDGET_H
