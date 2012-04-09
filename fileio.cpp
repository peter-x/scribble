#include "fileio.h"

#include "filelocker.h"
#include "zlib.h"

QByteArray FileIO::readGZFileLocked(const QFile &file)
{
    FileLocker locker(file);

    gzFile f = gzopen(file.fileName().toLocal8Bit().constData(), "r");
    if (f == 0)
        return QByteArray();

    QByteArray data;

    char buffer[1024];
    while (!gzeof(f)) {
        int len = gzread(f, buffer, 1024);
        if (len < 0) {
            gzclose(f);
            return QByteArray();
        }
        data.append(buffer, len);
    }
    gzclose(f);

    return data;
}

bool FileIO::writeGZFileLocked(const QFile &file, const QByteArray &data)
{
    FileLocker locker(file);

    gzFile f = gzopen(file.fileName().toLocal8Bit().constData(), "w");
    if (f == 0) {
        return false;
    }
    gzwrite(f, data.data(), data.size());
    gzclose(f);
    return true;
}
