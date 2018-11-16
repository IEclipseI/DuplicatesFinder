#ifndef STOPSEARCH_H
#define STOPSEARCH_H

#include <QDialog>
#include <QThread>

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    SearchDialog(QThread* searchThread, QWidget *parent = 0);
    ~SearchDialog();

public slots:
    void stopSearch();
    void updateBar(int);
    void setBarRange(int);
private:
    Ui::SearchDialog *ui;
    QThread* searchThread;
};

#endif // STOPSEARCH_H
