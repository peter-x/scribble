#ifndef FILEIO_H
#define FILEIO_H

#include <QByteArray>
#include <QFile>

class FileIO
{
public:
    static QByteArray readGZFileLocked(const QFile &file);
    static bool writeGZFileLocked(const QFile &file, const QByteArray &data);
};

#endif // FILEIO_H
