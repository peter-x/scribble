#ifndef ASYNCIO_H
#define ASYNCIO_H

#include <QThread>
#include <QFile>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>

#include "scribble_document.h"

/* Thread that is used to write data to a file asynchronously.
 * At any time, there is at most one writing operation pending
 * and any operation is allowed to fail. */
class AsyncWriter : public QThread
{
    Q_OBJECT
public:
    explicit AsyncWriter(QObject *parent = 0);
    ~AsyncWriter();

    void writeData(const QList<ScribblePage> &data, const QFile &file);
    void stopWriting();

signals:

protected:
    void run();

private:
    bool abort;
    bool waiting;
    QFile file;
    QList<ScribblePage> data;

    QMutex mutex;
    QWaitCondition workToDo;
    QWaitCondition workFinished;
};

#endif // ASYNCIO_H
