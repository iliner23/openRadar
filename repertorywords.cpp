#include "repertorywords.h"

repertoryWords::repertoryWords(QObject *parent) : QAbstractListModel(parent)
{

}
repertoryWords::repertoryWords(const QDir & wordFile, QObject * parent): QAbstractListModel(parent){
    setWordFile(wordFile);
}
void repertoryWords::setWordFile(const QDir &wordFile){

}
QVariant repertoryWords::data(const QModelIndex &index, int role) const{
    return QVariant();//NOTE : plug
}
bool repertoryWords::setData(const QModelIndex &index, const QVariant &value, int role){
    return true;//NOTE : plug
}
