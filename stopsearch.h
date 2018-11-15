#ifndef STOPSEARCH_H
#define STOPSEARCH_H

#include <QDialog>

namespace Ui {
class StopSearch;
}

class StopSearch : public QDialog
{
    Q_OBJECT

public:
    explicit StopSearch(QWidget *parent = 0);
    ~StopSearch();

private:
    Ui::StopSearch *ui;
};

#endif // STOPSEARCH_H
