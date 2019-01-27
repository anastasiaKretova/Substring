#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "scanner.h"
#include "worker.h"

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <memory>

namespace Ui {
    class MainWindow;
}

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit main_window(QWidget *parent = nullptr);
    ~main_window();

public slots:
    void beginInterface();
    void preIndexInterface();
    void postIndexInterface();
    void preScanInterface();
    void postScanInterface();
    void selectDirectory();
    void showAboutDialog();
    void setProgress(qint8 progress);
    void openFile(QTreeWidgetItem *, int);
    void searchSubstring();
    void newFile(const QString &);
    void deleteFiles();
    void stopScanning();
signals:
    void setPattern();
    void doSearch(const QString &);
    void stopWorker();

private:
    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<Worker> worker;
    QThread *thread;

    void resetThread();
    void setupInterface();
};

#endif // MAINWINDOW_H
