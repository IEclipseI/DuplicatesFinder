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
    connect(ui->collapseAll, SIGNAL(clicked()), this, SLOT(collapseAll()));
    connect(ui->expandAll, SIGNAL(clicked()), this, SLOT(expandAll()));
    connect(ui->autoselect, SIGNAL(clicked()), this, SLOT(autoselect()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteButton()));
    connect(ui->inputDirectoryName, SIGNAL(textChanged(const QString&)), this, SLOT(inputDirectoryNameTextChanged(const QString &)));
    connect(ui->chooseDirectoryButton, SIGNAL(clicked()), this, SLOT(chooseDir()));
    connect(ui->addToSearchButton, SIGNAL(clicked()), this, SLOT(addToSearch()));
    connect(ui->removeFromListButton, SIGNAL(clicked()), this, SLOT(removeFromList()));
    connect(ui->directoryList, SIGNAL(itemSelectionChanged()), this, SLOT(directoryListItemSelectionChanged()));
    connect(ui->searchButton, SIGNAL(clicked()), this, SLOT(search()));

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::collapseAll() {
    ui->duplicates->collapseAll();
}

void MainWindow::expandAll() {
    ui->duplicates->expandAll();
}

void MainWindow::autoselect() {
    for (int i = 0; i < ui->duplicates->topLevelItemCount(); ++i) {
        auto item = ui->duplicates->topLevelItem(i);
        for (int j = 1; j < item->childCount(); ++j) {
            item->child(j)->setSelected(true);
        }
    }
}

void MainWindow::inputDirectoryNameTextChanged(const QString &arg1) {
    if (QDir(arg1).exists()) {
        ui->addToSearchButton->setEnabled(true);
    } else {
        ui->addToSearchButton->setEnabled(false);
    }
}

void MainWindow::chooseDir() {
    QString directory = QFileDialog::getExistingDirectory(this);
    if (!directory.isEmpty()) ui->inputDirectoryName->setText(directory);
}

void MainWindow::addToSearch() {
    for (int i = ui->directoryList->topLevelItemCount() - 1; i >= 0; i--) {
        QTreeWidgetItem *item = ui->directoryList->topLevelItem(i);
        if (item->text(1) == ui->inputDirectoryName->text() ||
            ui->inputDirectoryName->text().indexOf(item->text(1)) >= 0) {
            ui->inputDirectoryName->clear();
            return;
        } else if (item->text(1).indexOf(ui->inputDirectoryName->text()) >= 0) {
            delete ui->directoryList->takeTopLevelItem(i);
        }
    }
    auto item = new QTreeWidgetItem(ui->directoryList);
    QDir d(ui->inputDirectoryName->text());
    item->setText(0, d.dirName());
    item->setText(1, d.path());
    ui->directoryList->addTopLevelItem(item);


    ui->statusBar->showMessage("\"" + ui->inputDirectoryName->text() + "\"" + " added to list", 3000);
    ui->inputDirectoryName->clear();
}

void MainWindow::removeFromList() {
    qDeleteAll(ui->directoryList->selectedItems());
}

void MainWindow::directoryListItemSelectionChanged() {
    ui->removeFromListButton->setEnabled(ui->directoryList->selectedItems().length() != 0);
}

void MainWindow::search() {
    ui->duplicates->clear();
    QVector<QString> dirList;
    for (int i = 0; i < ui->directoryList->topLevelItemCount(); i++) {
        dirList.push_back(ui->directoryList->topLevelItem(i)->text(1));
    }
    auto *finder_thread = new QThread();
    auto *finder = new same_file_finder(dirList);
    auto *searchDialog = new SearchDialog(finder_thread, this);
    finder->moveToThread(finder_thread);
    connect(finder_thread, SIGNAL(started()), finder, SLOT(findDuplicates()));
    connect(finder, SIGNAL(filesChecked(int)), searchDialog, SLOT(updateBar(int)));
    connect(finder, SIGNAL(filesToCheckCounted(int)), searchDialog, SLOT(setBarRange(int)));

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
