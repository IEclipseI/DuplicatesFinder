#ifndef CONFIRMDELETING_H
#define CONFIRMDELETING_H

#include <QDialog>

//#include "mainwindow.h"


namespace Ui {
class ConfirmDeleting;
}

class ConfirmDeleting : public QDialog
{
    Q_OBJECT
    friend class MainWindow;
public:
    explicit ConfirmDeleting(QWidget *parent = 0);
    ~ConfirmDeleting();

private:
    Ui::ConfirmDeleting *ui;
};

#endif // CONFIRMDELETING_H
