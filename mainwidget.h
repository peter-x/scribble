#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtGui>

#include "onyx/ui/status_bar.h"

#include "scribblearea.h"
#include "scribble_document.h"

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = 0);
    void loadFile(const QFile&);
    void saveFile(const QFile&);

signals:

public slots:
    void save();

private slots:
    void touchEventDataReceived(TouchData &);
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

    void updateProgressBar(int currentPage, int maxPages, int currentLayer, int maxLayers);
    void setPage(int percentage, int page);


protected:
    void keyPressEvent(QKeyEvent *);

private:
    TouchEventListener touchListener;
    int pressure_of_last_point_;

    QFile currentFile;
    ScribbleArea *scribbleArea;
    ScribbleDocument *document;

    ui::StatusBar *statusBar;
};

#endif // MAINWIDGET_H
