#include "searchdialog.h"
#include "ui_searchdialog.h"

SearchDialog::SearchDialog(QThread* searchThread, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog),
    searchThread(searchThread)
{
    ui->setupUi(this);
    connect(ui->stopSearchButton, SIGNAL(clicked()), this, SLOT(stopSearch()));
    ui->progressBar->setValue(0);
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::stopSearch() {
    this->done(0);
    searchThread->requestInterruption();
}

void SearchDialog::updateBar(int value) {
    ui->progressBar->setValue(value);
}


void SearchDialog::setBarRange(int b){
    ui->progressBar->setMaximum(b);
}