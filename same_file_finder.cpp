//
// Created by roman on 16.11.18.
//

#include <QtCore/QCryptographicHash>
#include <QtCore/QFile>
#include <QtWidgets/QTreeWidgetItem>
#include <QtCore/QDirIterator>
#include <QtWidgets/QDialog>
#include "same_file_finder.h"
#include <openssl/sha.h>

same_file_finder::same_file_finder(QVector<QString> &dirList, QObject *parent) : QObject(parent), dirList(dirList) {}

void same_file_finder::resolveInterruptionRequest() {
    if (QThread::currentThread()->isInterruptionRequested())
        throw std::exception();
}

void same_file_finder::findDuplicates() {
    auto groups = findDuplicatesImpl();
    emit searchResult(groups);
    emit searchEnds(QDialog::Accepted);
}

QVector<QVector<QString>> same_file_finder::findDuplicatesImpl() {
    QVector<QVector<QString>> groups;
    try {
        QVector<QPair<qint64, QString>> files;
        for (auto &directory : dirList) {
            QDirIterator it(directory,QDir::Hidden | QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                auto file = it.next();
                if(!it.fileInfo().isSymLink())
                    files.push_back(QPair<qint64, QString>(it.fileInfo().size(), file));
                resolveInterruptionRequest();
            }
        }
        emit filesToCheckCounted(files.size());
        std::sort(files.begin(), files.end(),
                  [](QPair<qint64, QString> &a, QPair<qint64, QString> &b) { return a.first > b.first; });
        int ind = 0;
        while (ind != files.size()) {
            resolveInterruptionRequest();
            int j = ind;
            while (j < files.size() && files[j].first == files[ind].first) {
                ++j;
            }
            if (j - ind > 1) {
                QHash<QByteArray, QVector<QString>> hashToFilesMap;
//                QCryptographicHash hashF(QCryptographicHash::Sha256);
                const int buffer_size = 1 << 18;
                char buffer[buffer_size];
                for (auto fileIt = files.begin() + ind; fileIt < files.begin() + j; fileIt++) {
                    QFile f(fileIt->second);
                    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        qint64 charRead = 0;
                        unsigned char digest[SHA256_DIGEST_LENGTH];
                        char digest1[SHA256_DIGEST_LENGTH];
                        SHA256_CTX ctx;
                        SHA256_Init(&ctx);
                        while ((charRead = f.read(buffer, buffer_size)) > 0) {
//                            hashF.addData(buffer, static_cast<int>(charRead));
                            SHA256_Update(&ctx, buffer, static_cast<size_t>(charRead));
                        }
                        SHA256_Final(digest, &ctx);
                        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
                            digest1[i] = digest[i];
                        }
                        QByteArray localResult(digest1, SHA256_DIGEST_LENGTH); //= /*hashF.result()*/;
//                        hashF.reset();
                        auto it = hashToFilesMap.find(localResult);
                        if (it != hashToFilesMap.end()) {
                            it->push_back(fileIt->second);
                        } else {
                            hashToFilesMap.insert(localResult, QVector<QString>({fileIt->second}));
                        }
                    }
                    resolveInterruptionRequest();
                }

                for (auto &hashKey : hashToFilesMap) {
                    if (hashKey.size() > 1) {
                        groups.push_back(QVector<QString>());
                        for (auto &filePath : hashKey) {
                            groups[groups.size() - 1].push_back(filePath);
                        }
                    }
                }
            }
            ind = j;
            emit filesChecked(ind);
        }
    } catch (...) {
        //no operations
    }
    return groups;
}
