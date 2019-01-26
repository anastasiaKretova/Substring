#ifndef SCANNER_H
#define SCANNER_H

#include <QObject>
#include <QVector>

#include "indexer.h"

using PatternTrigrams = QVector<qint32>;

class Scanner : public QObject {
    Q_OBJECT

public :
    Scanner(QString const &dir, FilesTrigrams *filesTrigrams);
    ~Scanner();

    void searchPattern();

public slots:
    void stop();

signals:
    void started();
    void finished();
    void interrupted();
    void updateProgress(qint8);
    void newFile(const QString &);

private:
    qint64 size = 0;
    void progress(qint64 curSize, qint8 curPercent);
    bool checkFile(QFile &file);

    QString stringPattern;
    char *pattern;
    FilesTrigrams *filesTrigrams;
    PatternTrigrams patternTrigrams;
    qint32 getTrigram(char *pointer);
    bool needStop;
};

#endif // SCANNER_H
