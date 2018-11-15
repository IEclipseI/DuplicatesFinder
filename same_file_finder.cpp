//
// Created by roman on 16.11.18.
//

#include <QtCore/QCryptographicHash>
#include <QtCore/QFile>
#include <QtWidgets/QTreeWidgetItem>
#include <QtCore/QDirIterator>
#include "same_file_finder.h"

same_file_finder::same_file_finder(QVector<QString> &dirList, MainWindow *mW): QObject(nullptr), dirList(dirList), mW(mW) {}

void same_file_finder::findDuplicates() {
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
                    auto *item = new QTreeWidgetItem(mW->ui->duplicates);
                   mW->ui->duplicates->addTopLevelItem(item);
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