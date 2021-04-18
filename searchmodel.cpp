#include "searchmodel.h"

searchModel::searchModel(QObject * parent): QAbstractListModel(parent)
{

}
void searchModel::setCatalogFile(const QDir & file, QByteArray pos){
    _data.clear();
    auto codec = QTextCodec::codecForName("system");

    _file = file;
    _db.open(_file.path().toStdString());
    _db.setIndex(4);

    auto iter = QByteArray::fromStdString(_db.at(pos.toStdString()));

    auto decode = [&](){
        const auto firstIt = iter.indexOf('\0', _db.serviceDataLenght());
        const auto secondIt = iter.indexOf('\0', firstIt + 1);

        QString tmpStr = iter.mid(_db.serviceDataLenght(), firstIt);
        tmpStr += "\n" + codec->toUnicode(iter.mid(firstIt + 1, secondIt - firstIt - 1));
        return tmpStr;
    };

    if(iter.left(6) == QByteArray(6, '\0')){
        _db.close();
        return;
    }

    _data.push_back(decode());
    const auto size = qFromLittleEndian<quint16>(iter.mid(24, 2).toStdString().c_str());

    pos = pos.right(6);
    pos += pos;

    for(auto i = 9; i != 5; --i){
        if(std::numeric_limits<char>::max() != pos.at(i)){
            pos[i] = pos.at(i) + 1;
            break;
        }
        else if(i == 3)
            throw std::runtime_error("incorrect key value");
    }

    iter = QByteArray::fromStdString(_db.at(pos.toStdString()));

    auto rev = pos;
    std::reverse(rev.begin(), rev.end());

    for(quint16 i = 0; i != size; ++i){
        iter = QByteArray::fromStdString(_db.next());

        if(iter.mid(12, 6) != rev.right(6))
            break;

        _data.push_back(decode());
    }

    _fpos = pos;
    emit dataChanged(QModelIndex(), QModelIndex());
}
QDir searchModel::catalogFile() const noexcept{
    return _file;
}
QVariant searchModel::data(const QModelIndex &index, int role) const{
    if(!index.isValid() || role != Qt::DisplayRole || _data.isEmpty())
        return QVariant();

    return _data.at(index.row());
}
int searchModel::rowCount(const QModelIndex &parent) const{
    Q_UNUSED(parent);
    return _data.size();
}
