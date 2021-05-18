#ifndef REPCHOSE_H
#define REPCHOSE_H

#include <QtWidgets>
#include "levels.h"
#include "openctree.h"

namespace Ui {
class RepChose;
}

class RepChose : public QDialog
{
    Q_OBJECT

public:
    explicit RepChose(const QStringList &, QWidget *parent = nullptr);
    ~RepChose();

private:
    Ui::RepChose *ui;
    QStringList _rLevels;
    Levels * _levels;
    quint16 _repLevel = -1;
public slots:
    void accept();
private slots:
    void finding(const QString &);
    void activateLevel(QListWidgetItem *);
    void showLevels();
    void sendLevel(const quint16);
signals:
    void chooseRep(QListWidgetItem *, quint16);
};

#endif // REPCHOSE_H
