#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "confirmdeleting.h"
#include "ui_confirmdeleting.h"
#include "stopsearch.h"
#include "ui_stopsearch.h"

#include <QDir>
#include <QFileDialog>
#include <iostream>
#include <QDirIterator>
#include <QCryptographicHash>
#include <QHash>
#include <QtCore/QThread>

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->removeFromListButton->setEnabled(false);
    ui->directoryList->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->directoryList->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    connect(ui->collapseAll, &QPushButton::clicked, this, &MainWindow::collapseAll);
    connect(ui->expandAll, &QPushButton::clicked, this, &MainWindow::expandAll);
    connect(ui->autoselect, &QPushButton::clicked, this, &MainWindow::autoselect);
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::deleteButton);

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::collapseAll() {
    for (int i = 0; i < ui->duplicates->topLevelItemCount(); ++i) {
        ui->duplicates->topLevelItem(i)->setExpanded(false);
    }

}

void MainWindow::expandAll() {
    for (int i = 0; i < ui->duplicates->topLevelItemCount(); ++i) {
        ui->duplicates->topLevelItem(i)->setExpanded(true);
    }
}

void MainWindow::autoselect() {
    for (int i = 0; i < ui->duplicates->topLevelItemCount(); ++i) {
        auto item = ui->duplicates->topLevelItem(i);
        for (int j = 1; j < item->childCount(); ++j) {
            item->child(j)->setSelected(true);
        }
    }
}

void MainWindow::on_inputDirectoryName_textChanged(const QString &arg1) {
    if (QDir(arg1).exists()) {
        ui->addToSearchButton->setEnabled(true);
    } else {
        ui->addToSearchButton->setEnabled(false);
    }
}

void MainWindow::on_chooseDirectoryButton_clicked() {
    QString directory = QFileDialog::getExistingDirectory(this);
    if (!directory.isEmpty()) ui->inputDirectoryName->setText(directory);
}

void MainWindow::on_addToSearchButton_clicked() {
    for (int i = ui->directoryList->topLevelItemCount() - 1; i >= 0; i--) {
        QTreeWidgetItem *item = ui->directoryList->topLevelItem(i);
        if (item->text(1) == ui->inputDirectoryName->text() ||
            ui->inputDirectoryName->text().indexOf(item->text(1)) >= 0) {
            ui->inputDirectoryName->clear();
            return;
        } else if (item->text(1).indexOf(ui->inputDirectoryName->text()) >= 0) {
            try {
                delete ui->directoryList->takeTopLevelItem(i);
            } catch (...) {

            }
        }
    }
    auto item = new QTreeWidgetItem(ui->directoryList);
    QDir d(ui->inputDirectoryName->text());
    item->setText(0, d.dirName());
    item->setText(1, d.path());
    ui->directoryList->addTopLevelItem(item);


    ui->statusBar->showMessage("\"" + ui->inputDirectoryName->text() + "\"" + " added to list");
    ui->inputDirectoryName->clear();
}

void MainWindow::on_removeFromListButton_clicked() {
    qDeleteAll(ui->directoryList->selectedItems());
}

void MainWindow::on_directoryList_itemSelectionChanged() {
    ui->removeFromListButton->setEnabled(ui->directoryList->selectedItems().length() != 0);
}

void MainWindow::on_searchButton_clicked() {
    ui->duplicates->clear();
    auto *stopSearchThread = new QThread();
    auto *stopSearch = new StopSearch();
    QVector<QString> dirList;
    for (int i = 0; i < ui->directoryList->topLevelItemCount(); i++) {
        dirList.push_back(ui->directoryList->topLevelItem(i)->text(1));
    }
    QVector<QPair<qint64, QString>> files;
    for (auto &directory : dirList) {
        QDirIterator it(directory, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            files.push_back(QPair<qint64, QString>(it.fileInfo().size(), it.next()));
        }
    }
    std::sort(files.begin(), files.end(),
              [](QPair<qint64, QString> &a, QPair<qint64, QString> &b) { return a.first < b.first; });

    int ind = 0;
    size_t groupId = 1;
    size_t files_found = 0;
    while (ind != files.size()) {
        int j = ind;
        while (j < files.size() && files[j].first == files[ind].first) {
            ++j;
        }
        if (j - ind > 1) {
            QHash<QByteArray, QVector<QString>> hashToFilesMap;
            QCryptographicHash hashF(QCryptographicHash::Sha256);
            const int buffer_size = 1 << 18;
            char buffer[buffer_size];
            for (auto fileIt = files.begin() + ind; fileIt < files.begin() + j; fileIt++) {
                QFile f(fileIt->second);
                if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    qint64 charRead = 0;
                    while ((charRead = f.read(buffer, buffer_size)) > 0) {
                        hashF.addData(buffer, static_cast<int>(charRead));
                    }
                    QByteArray localResult = hashF.result();
                    hashF.reset();
                    auto it = hashToFilesMap.find(localResult);
                    if (it != hashToFilesMap.end()) {
                        it->push_back(fileIt->second);
                    } else {
                        hashToFilesMap.insert(localResult, QVector<QString>({fileIt->second}));
                    }
                }
            }

            for (auto &hashKey : hashToFilesMap) {
                if (hashKey.size() > 1) {
                    files_found += hashKey.size();
                    auto *item = new QTreeWidgetItem(ui->duplicates);
                    ui->duplicates->addTopLevelItem(item);
                    for (auto &filePath : hashKey) {
                        QTreeWidgetItem *itemChild = new QTreeWidgetItem(item);
                        itemChild->setText(0, filePath);
                        item->addChild(itemChild);
                    }
                    item->setText(0, "№" + QString::number(groupId++) + ", " + QString::number(hashKey.size()) +
                                     " files in group");
                    item->setExpanded(false);
                }
            }
        }
        ind = j;
    }
    groupId--;

}

void MainWindow::deleteButton() {
    auto d = new ConfirmDeleting(this);
    d->ui->label->setText("Are you sure to delete selected files?");
    if (d->exec() == QDialog::Accepted) {
        auto selected = ui->duplicates->selectedItems();
        for (auto &item : selected) {
            if (item->parent() != nullptr) {
                if (QFile::remove(item->text(0)))
                    delete item;
            }
        }
    }
}
