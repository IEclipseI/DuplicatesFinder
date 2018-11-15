#include "stopsearch.h"
#include "ui_stopsearch.h"

StopSearch::StopSearch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StopSearch)
{
    ui->setupUi(this);
}

StopSearch::~StopSearch()
{
    delete ui;
}
