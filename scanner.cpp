#include "scanner.h"

#include <QString>
#include <QFileInfo>
#include <QFile>
#include <iostream>

const qint32 SHIFT = 2;
const qint32 MAX_CHAR = 256;
const qint64 BUFFER_SIZE = 128*1024;

Scanner::Scanner(QString const & stringPattern, FilesTrigrams *filesTrigrmas) {
    this->stringPattern = stringPattern;
    this->filesTrigrams = filesTrigrmas;
    this->needStop = false;
    pattern = new char[static_cast<qint32>(stringPattern.size()) + 1];
    memcpy(pattern, stringPattern.toLatin1().data(), stringPattern.size());
    pattern[stringPattern.size()] = '\0';
    for (qint32 i = 0; i < (qint32)(stringPattern.size()) - SHIFT; ++i) {
        patternTrigrams.push_back(getTrigram(pattern + i));
    }
}

Scanner::~Scanner() {
    delete[] pattern;
}

qint32 Scanner::getTrigram(char * pointer) {
    qint32 result = 0;
    for (int i = 0; i < 3; i++) {
        result = result * MAX_CHAR + qint32(pointer[i]);
    }
    return result;
}

void Scanner::searchPattern() {
    emit started();
    for (auto path: filesTrigrams->keys()) {
        if (needStop) {
            break;
        }
        QFileInfo info(path);
        size += info.size();
    }
    if (size == 0) {
        emit updateProgress(100);
        return;
    }

    qint64 curSize = 0;
    qint8 curPercent = 0;
    for (auto path: filesTrigrams->keys()) {
        if (needStop) {
            break;
        }
        QFileInfo info(path);
        curSize += info.size();

        QFile file(path);
        if (checkFile(file)) {
            emit newFile(file.fileName());
        }
        progress(curSize, curPercent);
    }
    if (needStop) {
        emit interrupted();
    } else {
        emit finished();
    }
}

bool Scanner::checkFile(QFile &file) {
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::logic_error("Can't open file: " + file.fileName().toStdString());
    }

    qint64  patternShift = qint64(stringPattern.size()) - 1;
    char *buffer = new char[BUFFER_SIZE + 1];
    buffer[BUFFER_SIZE] = '\0';
    file.read(buffer, patternShift);
    while (!file.atEnd()) {
        if (needStop) {
            break;
        }
        qint64 size = patternShift + file.read(buffer + patternShift, BUFFER_SIZE - patternShift);
        buffer[size] = '\0';
        char *ptr = strstr(buffer, pattern);
        if (ptr) {
            file.close();
            delete[] buffer;
            return 1;
        }
    }
    file.close();
    delete[] buffer;
    return 0;
}

void Scanner::progress(qint64 curSize, qint8 curPercent) {
    qint8 percent = curSize / size * 100;
    if (percent > curPercent) {
        curPercent = percent;
        emit updateProgress(percent);
    }
}

void Scanner::stop() {
    needStop = true;
}
