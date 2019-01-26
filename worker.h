#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QString>

#include "indexer.h"
#include "scanner.h"

class Worker : public QObject {
    Q_OBJECT

public:
    Worker(QString const &dir, QObject *parent);
    ~Worker() = default;

signals:
    void stopEverything();
public slots:
    void updateFile(const QString &);
    void indexDirectory();
    void scanDirectory() ;
    void newPattern(const QString &pattern);
    void stop();

private:
    QObject *mainWindow;
    QString dir;
    QString pattern;
    FilesTrigrams filesTrigrams;
    QFileSystemWatcher *watcher;
};

#endif // WORKER_H
