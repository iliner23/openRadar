#include "proxysearchmodel.h"

proxySearchModel::proxySearchModel(QObject *parent) : QSortFilterProxyModel(parent)
{

}
void proxySearchModel::setRootIndex(const QModelIndex &index){
   if(index.isValid())
        _root = index;
   else
       _root = QModelIndex();

   invalidateFilter();
}
QModelIndex proxySearchModel::rootIndex() const{
    if(_root.isValid())
        return _root;

    return QModelIndex();
}
bool proxySearchModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const{
    if(mapFromSource(sourceParent) == _root){
        auto index = sourceModel()->index(sourceRow, 0, sourceParent);

        if(sourceModel()->data(index).toString().contains(filterRegExp()))
            return true;

        return false;
    }

    return true;
}
