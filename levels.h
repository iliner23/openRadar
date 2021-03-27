#ifndef LEVELS_H
#define LEVELS_H

#include <QDialog>
#include <QTextCodec>
#include <vector>
#include "openctree.h"

namespace Ui {
class Levels;
}

class Levels : public QDialog
{
    Q_OBJECT

public:
    explicit Levels(QWidget *parent = nullptr);
    void showLevels(const QString &);
    ~Levels();
public slots:
    void reject();
    void accept();
signals:
    void sendLevel(quint16);
private:
    Ui::Levels *ui;
};

#endif // LEVELS_H
