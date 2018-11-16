#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class same_file_finder;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_inputDirectoryName_textChanged(const QString &arg1);

    void on_chooseDirectoryButton_clicked();

    void on_addToSearchButton_clicked();

    void on_removeFromListButton_clicked();

    void on_directoryList_itemSelectionChanged();

    void on_searchButton_clicked();

    void collapseAll();

    void expandAll();

    void autoselect();

    void deleteButton();

    void showResults(QVector<QVector<QString>>);
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
