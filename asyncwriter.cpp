#include "asyncwriter.h"

#include <QMutexLocker>

#include "fileio.h"

AsyncWriter::AsyncWriter(QObject *parent) :
    QThread(parent), abort(false), waiting(false)
{
}

AsyncWriter::~AsyncWriter()
{
    mutex.lock();
    abort = true;
    workToDo.wakeOne();
    mutex.unlock();

    wait();
}

void AsyncWriter::writeData(const QList<ScribblePage> &data, const QFile &file)
{
    QMutexLocker locker(&mutex);

    this->data = data;
    this->file.setFileName(file.fileName());

    if (!isRunning()) {
        start();
    } else {
        workToDo.wakeOne();
    }
}

void AsyncWriter::stopWriting()
{
    QMutexLocker locker(&mutex);

    file.setFileName(QString());

    if (!waiting && isRunning())
        workFinished.wait(&mutex, 3000);
}

void AsyncWriter::run()
{
    forever {
        mutex.lock();
        QFile f;
        f.setFileName(file.fileName());
        const QList<ScribblePage> d = data;
        mutex.unlock();

        if (abort) {
            workFinished.wakeAll();
            return;
        }

        if (!f.fileName().isEmpty()) {
            QByteArray output = ScribbleDocument::toXournalXMLFormat(d);
            FileIO::writeGZFileLocked(f, output);
        }

        mutex.lock();
        file.setFileName(QString());
        if (abort) {
            workFinished.wakeAll();
            mutex.unlock();
            return;
        }
        waiting = true;
        workFinished.wakeAll();
        workToDo.wait(&mutex);
        waiting = false;
        mutex.unlock();
    }
}
