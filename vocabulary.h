#ifndef VOCABULARY_H
#define VOCABULARY_H

#include <QDialog>

namespace Ui {
class vocabulary;
}

class vocabulary : public QDialog
{
    Q_OBJECT

public:
    explicit vocabulary(QWidget *parent = nullptr);
    ~vocabulary();

private:
    Ui::vocabulary *ui;
};

#endif // VOCABULARY_H
