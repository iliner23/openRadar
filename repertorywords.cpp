#include "repertorywords.h"

repertoryWords::repertoryWords(QObject *parent) : QAbstractListModel(parent)
{

}
repertoryWords::repertoryWords(const QDir & wordFile, QObject * parent): QAbstractListModel(parent){
    setWordFile(wordFile);
}
void repertoryWords::setWordFile(const QDir &wordFile){
    beginResetModel();
    _word.open(wordFile.filePath("word1").toStdString());
    endResetModel();
}
QVariant repertoryWords::data(const QModelIndex &index, int role) const{
    if(!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    return _words.at(index.row());
}
int repertoryWords::rowCount(const QModelIndex &parent) const{
    Q_UNUSED(parent);
    return _words.size();
}
