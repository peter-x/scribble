#ifndef FILELOCKER_H
#define FILELOCKER_H

#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QSet>

class FileLockerManager
{
public:
    FileLockerManager() {
    }

    void lockFile(QString file) {
        QMutexLocker locker(&mutex);
        lockedFiles.insert(file);
    }
    void unlockFile(QString file) {
        QMutexLocker locker(&mutex);
        lockedFiles.remove(file);
    }

    static FileLockerManager &instance() {
        static FileLockerManager manager;
        return manager;
    }

private:
    QSet<QString> lockedFiles;
    QMutex mutex;

    Q_DISABLE_COPY(FileLockerManager)
};

class FileLocker
{
public:
    inline explicit FileLocker(QString file) {
        lockedFile = file;
        lock();
    }

    inline explicit FileLocker(const QFile &file) {
        lockedFile = QFileInfo(file).absoluteFilePath();
        lock();
    }

    inline ~FileLocker() {
        unlock();
    }
private:
    inline void lock() {
        FileLockerManager::instance().lockFile(lockedFile);
    }
    inline void unlock() {
        FileLockerManager::instance().unlockFile(lockedFile);
    }

    QString lockedFile;
    Q_DISABLE_COPY(FileLocker)
};

#endif // FILELOCKER_H
