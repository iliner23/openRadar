#ifndef KEYCHOOSE_H
#define KEYCHOOSE_H

#include <QtWidgets>
#include "languages.h"
#include "openctree.h"
#include "keysremedlist.h"

namespace Ui {
class KeyChoose;
}

class KeyChoose : public QDialog
{
    Q_OBJECT

public:
    explicit KeyChoose(const QStringList, QWidget *parent = nullptr);
    keysRemedList * remedList() const { return _remedList; }
    ~KeyChoose();

private:
    Ui::KeyChoose *ui;
    keysRemedList * _remedList;

    QStringListModel * _model;
    QSortFilterProxyModel * _proxyModel;
    QTextCodec * _codec = nullptr;
private slots:
    void finding(const QString);
    void activateLevel(const QModelIndex &);
public slots:
    void accept();
signals:
    void openRemedList(int);
};

#endif // KEYCHOOSE_H
