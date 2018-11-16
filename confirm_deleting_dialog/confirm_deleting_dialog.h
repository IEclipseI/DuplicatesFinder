#ifndef CONFIRMDELETING_H
#define CONFIRMDELETING_H

#include <QDialog>

//#include "mainwindow.h"


namespace Ui {
class ConfirmDeletingDialog;
}

class ConfirmDeletingDialog : public QDialog
{
    Q_OBJECT
    friend class MainWindow;
public:
    explicit ConfirmDeletingDialog(QWidget *parent = 0);
    ~ConfirmDeletingDialog();

private:
    Ui::ConfirmDeletingDialog *ui;
};

#endif // CONFIRMDELETING_H
