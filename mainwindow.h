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
    void inputDirectoryNameTextChanged(const QString &arg1);
    void chooseDir();
    void addToSearch();
    void removeFromList();
    void directoryListItemSelectionChanged();
    void search();
    void collapseAll();
    void expandAll();
    void autoselect();
    void deleteButton();
    void showResults(QVector<QVector<QString>>);
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
