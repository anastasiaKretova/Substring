#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "worker.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QDesktopServices>
#include <iostream>
#include <QThread>

main_window::main_window(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      worker(nullptr),
      thread(nullptr)
{
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    //ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->setUniformRowHeights(1);

    QCommonStyle style;
    ui->actionScan_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));
    setWindowTitle(QString("Select directory to scan"));


    ui->statusBar->addPermanentWidget(ui->stopButton);
    ui->statusBar->addPermanentWidget(ui->deleteButton);
    ui->statusBar->addPermanentWidget(ui->progressBar);

    ui->progressBar->setHidden(true);
    ui->deleteButton->setHidden(true);
    ui->progressBar->setRange(0, 100);
    ui->stopButton->setHidden(true);

    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &main_window::searchSubstring);
    connect(ui->actionScan_Directory, &QAction::triggered, this, &main_window::selectDirectory);
    connect(ui->deleteButton, &QPushButton::clicked, this, &main_window::deleteFiles);
    connect(ui->stopButton, &QPushButton::clicked, this, &main_window::stopScanning);
    connect(ui->treeWidget,
                SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
                this,
                SLOT(openFile(QTreeWidgetItem*, int)));
}

main_window::~main_window()
{
    emit stopWorker();
    if (thread != nullptr) {
        thread->exit();
        thread->wait();
    }
    delete thread;
}

void main_window::selectDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    QFileInfo selectedDirInfo = QFileInfo(dir);
    if (!selectedDirInfo.exists()) {
        return;
    }
    setupInterface();
    setWindowTitle(dir);
    resetThread();
    auto worker = std::make_unique<Worker>(dir,this);
    //worker = new Worker(dir, this);
    thread = new QThread();
    worker->moveToThread(thread);


    connect(this, SIGNAL(stopWorker()), worker.get(), SLOT(stop()), Qt::DirectConnection);
    connect(thread, SIGNAL(started()), worker.get(), SLOT(indexDirectory()));
    //connect(thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    //connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(this, SIGNAL(doSearch(const QString&)), worker.get(), SLOT(newPattern(const QString&)));
    thread->start();
}

void main_window::showAboutDialog()
{
    QMessageBox::aboutQt(this);
}

void main_window::newFile(const QString & fileName) {
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, fileName);
    ui->treeWidget->addTopLevelItem(item);
}

void main_window::searchSubstring() {
    QString pattern = ui->lineEdit->text();
    emit doSearch(pattern);
}

void main_window::deleteFiles() {
    auto selectedItems = ui->treeWidget->selectedItems();
    auto answer = QMessageBox::question(this, "Deleting",
                                        "Do you want to delete " + QString::number(selectedItems.size()) + " files?");
    if (answer == QMessageBox::No) {
        return;
    }
    for (auto & selectedItem: selectedItems) {
        QFile file(selectedItem->text(0));
        if (!file.exists() || file.remove()) {
            delete selectedItem;
        }
    }
}

void main_window::openFile(QTreeWidgetItem* item, int) {
    QFile file(item->text(0));
    if (file.exists()) {
        QDesktopServices::openUrl(item->text(0));
    }
}

void main_window::preIndexInterface() {
    ui->progressBar->setHidden(false);
    ui->stopButton->setHidden(false);
    ui->progressBar->setValue(0);
}

void main_window::postIndexInterface() {
    ui->stopButton->setHidden(true);
    ui->progressBar->setValue(100);
}

void main_window::preScanInterface() {
    ui->progressBar->setHidden(false);
    ui->stopButton->setHidden(false);
    ui->deleteButton->setHidden(true);
    ui->treeWidget->clear();
    ui->progressBar->setValue(0);
}

void main_window::postScanInterface() {
    ui->stopButton->setHidden(true);
    ui->deleteButton->setHidden(false);
    ui->progressBar->setValue(100);
}

void main_window::setupInterface() {
    ui->treeWidget->clear();
    ui->stopButton->setHidden(true);
    ui->deleteButton->setHidden(true);
    ui->progressBar->setHidden(true);
    setWindowTitle(QString("Please select directory to scan"));
}

void main_window::setProgress(qint8 progress) {
    ui->progressBar->setValue(progress);
}

void main_window::resetThread() {
    emit stopWorker();
    if (thread != nullptr && !thread->isFinished()) {
        thread->quit();
        thread->wait();
    }
    //delete thread;
}

void main_window::stopScanning() {
    emit stopWorker();
}

void main_window::beginInterface() {
    setupInterface();
}
