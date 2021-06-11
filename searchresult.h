#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

#include <QDialog>

namespace Ui {
class searchResult;
}

class searchResult : public QDialog
{
    Q_OBJECT

public:
    explicit searchResult(QWidget *parent = nullptr);
    ~searchResult();

private:
    Ui::searchResult *ui;
};

#endif // SEARCHRESULT_H
