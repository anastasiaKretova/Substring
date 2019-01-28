#include "scanner.h"

#include <QString>
#include <QFileInfo>
#include <QFile>
#include <iostream>
#include <QLatin1String>

const qint32 SHIFT = 2;
const qint32 MAX_CHAR = 256;
const qint64 BUFFER_SIZE = 128*1024;

Scanner::Scanner(QString const & stringPattern, FilesTrigrams *filesTrigrmas) {
    this->stringPattern = stringPattern;
    this->filesTrigrams = filesTrigrmas;
    this->needStop = false;
    std::string k = stringPattern.toStdString();
    for (int i = 0; i < stringPattern.size(); ++i) {
        pattern.push_back(k[i]);
    }
    pattern.push_back('\0');
    for (qint32 i = 0; i < (qint32)(stringPattern.size()) - SHIFT; ++i) {
        patternTrigrams.push_back(getTrigram(pattern.data() + i));
    }
}

qint32 Scanner::getTrigram(char *pointer) {
    qint32 result = 0;
    for (int i = 0; i < 3; i++) {
        result = result * MAX_CHAR + qint32(pointer[i]);
    }
    return result;
}

void Scanner::searchPattern() {
    emit started();
    for (auto it = filesTrigrams->begin(); it != filesTrigrams->end(); it++) {
        if (needStop) {
            break;
        }
        size += it.value().fileSize;
    }
    if (size == 0) {
        emit updateProgress(100);
        return;
    }

    for (auto it = filesTrigrams->begin(); it != filesTrigrams->end(); it++) {
        if (needStop) {
            break;
        }
        curSize += it.value().fileSize;

        QFile file(it.key());
        if (checkFile(file, it.value())) {
            emit newFile(file.fileName());
        }
        progress();
    }
    if (needStop) {
        emit interrupted();
    } else {
        emit finished();
    }
}

bool Scanner::checkFile(QFile &file, FileTrigrams fileTrigrams) {
    for (auto trigram: patternTrigrams) {
        if (!fileTrigrams.trigrams.contains(trigram)) return 0;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        throw std::logic_error("Can't open file: " + file.fileName().toStdString());
    }

    qint64 patternShift = qint64(stringPattern.size()) - 1;
    std::vector<char> buffer(BUFFER_SIZE);
    file.read(buffer.data(), patternShift);
    while (!file.atEnd()) {
        if (needStop) {
            break;
        }
        qint64 size = patternShift + file.read(buffer.data() + patternShift, BUFFER_SIZE - patternShift);
        buffer.push_back('\0');
        char *ptr = strstr(buffer.data(), pattern.data());
        if (ptr) {
            return 1;
        }
    }
    return 0;
}

void Scanner::progress() {
    qint8 percent = static_cast<qint8>(100 * curSize / size);
    if (percent > curPercent) {
        curPercent = percent;
        emit updateProgress(percent);
    }
}

void Scanner::stop() {
    needStop = true;
}
