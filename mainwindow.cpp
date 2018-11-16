#include <QDir>
#include <QFileDialog>
#include <iostream>
#include <QDirIterator>
#include <QCryptographicHash>
#include <QHash>
#include <QtCore/QThread>
#include <QMetaType>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "confirm_deleting_dialog/confirm_deleting_dialog.h"
#include "confirm_deleting_dialog/ui_confirm_deleting_dialog.h"
#include "searchdialog/searchdialog.h"
#include "searchdialog/ui_searchdialog.h"
#include "same_file_finder.h"

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
    QVector<QString> dirList;
    for (int i = 0; i < ui->directoryList->topLevelItemCount(); i++) {
        dirList.push_back(ui->directoryList->topLevelItem(i)->text(1));
    }
    auto *finder_thread = new QThread();
    auto *finder = new same_file_finder(dirList);
    auto *searchDialog = new SearchDialog(finder_thread, this);
    finder->moveToThread(finder_thread);
    connect(finder_thread, &QThread::started, finder, &same_file_finder::findDuplicates);
    connect(finder, &same_file_finder::fileChecked, searchDialog, &SearchDialog::updateBar);
    connect(finder, &same_file_finder::filesToCheckCounted, searchDialog, &SearchDialog::setBarRange);
    qRegisterMetaType<QVector<QVector<QString>>>("QVector<QVector<QString>>");
    connect(finder, SIGNAL(searchResult(QVector<QVector<QString>>)), this, SLOT(showResults(
            QVector<QVector<QString>>)));

    connect(finder, SIGNAL(searchEnds(int)), finder_thread, SLOT(quit()));
    connect(finder, SIGNAL(searchEnds(int)), searchDialog, SLOT(done(int)));
    connect(finder, SIGNAL(searchEnds(int)), finder, SLOT(deleteLater()));
    connect(finder_thread, SIGNAL(finished()), finder_thread, SLOT(deleteLater()));
    finder_thread->start();
    if (searchDialog->exec() == QDialog::Rejected) {
        searchDialog->stopSearch();
    }
    delete searchDialog;
}

void MainWindow::deleteButton() {
    auto d = new ConfirmDeletingDialog(this);
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

void MainWindow::showResults(QVector<QVector<QString>> groups) {
    int groupId = 1;
    int duplCount = 0;
    for (auto group : groups) {
        auto *item = new QTreeWidgetItem(ui->duplicates);
        ui->duplicates->addTopLevelItem(item);
        duplCount += group.size();
        for (auto &filePath : group) {
            QTreeWidgetItem *itemChild = new QTreeWidgetItem(item);
            itemChild->setText(0, filePath);
            item->addChild(itemChild);
        }
        item->setText(0, "№" + QString::number(groupId++).append(", ").append(QString::number(group.size())).append(
                " files in group"));
        item->setExpanded(false);
    }
    ui->statusBar->showMessage(QString::number(duplCount).append(" files are founded"));
}
