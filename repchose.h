#ifndef REPCHOSE_H
#define REPCHOSE_H

#include <QtWidgets>
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
    quint16 _repLevel = -1;
    QSortFilterProxyModel * _proxyModel;
    QStringListModel * _model;
    QTextCodec * _codec = nullptr;
public slots:
    void accept();
private slots:
    void finding(const QString &);
    void activateLevel(const QModelIndex &);
    void showLevels();
    void sendLevel(const quint16);
    void setCodec();
signals:
    void chooseRep(QModelIndex &, quint16, QTextCodec *);
};

#endif // REPCHOSE_H
