//
// Created by roman on 16.11.18.
//

#ifndef HW5_FILE_FINDER_THREAD_H
#define HW5_FILE_FINDER_THREAD_H


#include <QtCore/QThread>

#include "mainwindow.h"
#include "ui_mainwindow.h"

class same_file_finder : public QObject {
    Q_OBJECT

    friend class MainWindow;
public:
    same_file_finder(QVector<QString> &dirList, QObject* parent = 0);

public slots:
    void findDuplicates();

signals:
    void searchEnds(int);
    void filesChecked(int);
    void filesToCheckCounted(int);
    void searchResult(QVector<QVector<QString>>);

private:
    QVector<QVector<QString>> findDuplicatesImpl();
    void resolveInterruptionRequest();
    QVector<QString> dirList;
//    MainWindow* mW;
};


#endif //HW5_FILE_FINDER_THREAD_H
