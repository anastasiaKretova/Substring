#ifndef INDEXER_H
#define INDEXER_H

#include <QFile>
#include <QFileSystemWatcher>
#include <QObject>
#include <QSet>
#include <QMap>

using FileTrigrams = QSet<qint32>;
using FilesTrigrams = QMap<QString, FileTrigrams>;

class Indexer : public QObject {
    Q_OBJECT

public:
    explicit Indexer(QString const &dir, QFileSystemWatcher *watcher);

    void indexDirectory(FilesTrigrams &filesTrigrams);
    void indexFile(QFile &file, FileTrigrams &fileTrigrams);
    qint32 hashTrigram(char *trigramPointer);


signals:
    void started();
    void finished();
    void interrupted();
    void updateProgress(qint8 progress);

public slots:
    void stop();

private:
    void progress(qint64 curSize, qint8 curPercent);
    QFileSystemWatcher *watcher;
    QString directory;
    qint64 size;
    bool needStop;
};

#endif // INDEXER_H