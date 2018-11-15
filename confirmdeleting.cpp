#include "confirmdeleting.h"
#include "ui_confirmdeleting.h"

ConfirmDeleting::ConfirmDeleting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfirmDeleting)
{
    ui->setupUi(this);
}

ConfirmDeleting::~ConfirmDeleting()
{
    delete ui;
}
