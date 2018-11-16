#include "confirm_deleting_dialog.h"
#include "ui_confirm_deleting_dialog.h"

ConfirmDeletingDialog::ConfirmDeletingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfirmDeletingDialog)
{
    ui->setupUi(this);
}

ConfirmDeletingDialog::~ConfirmDeletingDialog()
{
    delete ui;
}
