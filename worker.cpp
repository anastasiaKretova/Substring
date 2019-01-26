#include "worker.h"

#include <iostream>
#include <QFileInfo>

Worker::Worker(QString const &dir, QObject *parent) : mainWindow(parent), dir(dir) {
    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(fileChanged(const QString &)), this, SLOT(updateFile(const QString &)));
}

void Worker::indexDirectory() {
    auto files = watcher->files();
    if (!files.isEmpty()) {
        watcher->removePaths(files);
    }

    Indexer indexer(dir, watcher);

    connect(this, SIGNAL(stopEverything()), &indexer, SLOT(stop()), Qt::DirectConnection);
    connect(&indexer, SIGNAL(started()), mainWindow, SLOT(preIndexInterface()));
    connect(&indexer, SIGNAL(finished()), mainWindow, SLOT(postIndexInterface()));
    connect(&indexer, SIGNAL(interrupted()), mainWindow, SLOT(beginInterface()));
    connect(&indexer, SIGNAL(updateProgress(qint8)), mainWindow, SLOT(setProgress(qint8)));
    indexer.indexDirectory(filesTrigrams);
}

void Worker::scanDirectory() {
    Scanner scanner(pattern, &filesTrigrams);

    connect(&scanner, SIGNAL(started()), mainWindow, SLOT(preScanInterface()));
    connect(&scanner, SIGNAL(finished()), mainWindow, SLOT(postScanInterface()));
    connect(&scanner, SIGNAL(interrupted()), mainWindow, SLOT(postIndexInterface()));
    connect(&scanner, SIGNAL(updateProgress(qint8)), mainWindow, SLOT(setProgress(qint8)));

    try {
        scanner.searchPattern();
    } catch (std::logic_error) {

    }
}

void Worker::newPattern(const QString &pattern) {
    if (pattern.isEmpty()) {
        return;
    }
    if (this->pattern != pattern) {
        this->pattern = pattern;
    }
    scanDirectory();
}

void Worker::updateFile(const QString &path) {
    std::cout << "updating" << path.toStdString() << std::endl;

    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.permission(QFile::ReadUser)) {
        filesTrigrams.remove(path);
        watcher->removePath(path);
        return;
    }

    Indexer indexer(dir, watcher);
    filesTrigrams[path].clear();
    QFile file(path);
    FileTrigrams fileTrigrams;
    indexer.indexFile(file, fileTrigrams);
    filesTrigrams[path] = fileTrigrams;
}

void Worker::stop() {
    emit stopEverything();
}
