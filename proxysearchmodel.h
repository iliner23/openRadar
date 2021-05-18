#ifndef PROXYSEARCHMODEL_H
#define PROXYSEARCHMODEL_H

#include <QSortFilterProxyModel>

class proxySearchModel : public QSortFilterProxyModel
{
public:
    explicit proxySearchModel(QObject *parent = nullptr);
    void setRootIndex(const QModelIndex & index);
    QModelIndex rootIndex() const;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
private:
    QModelIndex _root;
};

#endif // PROXYSEARCHMODEL_H
