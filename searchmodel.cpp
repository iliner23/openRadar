#include "searchmodel.h"
searchModel::_node::_node(const QString & data, const QByteArray & key, bool marker, _node * parent){
    _data = data;
    _parent = parent;
    _marker = marker;
    _key = key;

    if(_parent != nullptr){
        _parent->_children.push_back(this);
        auto size = _parent->_children.size();
        _row = (size == 0) ? 0 : size - 1;
    }
}
searchModel::_node * searchModel::_node::child(int pos) const noexcept{
    if(pos >= 0 && pos < _children.size())
        return _children.at(pos);
    else
        return nullptr;
}
searchModel::_node::~_node(){
    if(!_children.empty()){
        for(auto & it : _children)
            delete it;
    }
}


searchModel::searchModel(const QDir & filename, const QByteArray & data, QObject * object): QAbstractItemModel(object){
    setCatalogFile(filename, data);
}
void searchModel::createHeap(_node * parent, QByteArray pos){
    auto codec = QTextCodec::codecForName("system");
    auto iter = QByteArray::fromStdString(_db.at(pos.toStdString()));

    auto decode = [&](){
        const auto firstIt = iter.indexOf('\0', _db.serviceDataLenght());
        const auto secondIt = iter.indexOf('\0', firstIt + 1);
        const auto localize = codec->toUnicode(iter.mid(firstIt + 1, secondIt - firstIt - 1));

        QString tmpStr = iter.mid(_db.serviceDataLenght(), firstIt);
        tmpStr += ((localize.isEmpty()) ? QString() : "\n" + localize);
        return tmpStr;
    };

    auto increase = [](auto & array, const auto startPos){
        for(auto i = startPos; i != 5; --i){
            if(std::numeric_limits<char>::max() != (uchar) array.at(i)){
                array[i] = ((uchar) array.at(i)) + 1;
                break;
            }
            else{
                if(i == 6)
                    Q_ASSERT("error increment key");

                array[i] = (uchar) 0;
                array[i - 1] = ((uchar) array.at(i - 1)) + 1;
            }
        }
    };

    if(iter.left(6) == QByteArray(6, '\0')){
        _db.close();
        return;
    }

    const auto size = qFromLittleEndian<quint16>(iter.mid(24, 2));

    pos = pos.right(6);
    pos += pos;

    QByteArray incTemp = pos;
    increase(incTemp, 11);

    if(_db.haveKey(incTemp.toStdString()))
        pos = std::move(incTemp);
    else
        increase(pos, 9);

    auto rev = pos.left(4);
    std::reverse(rev.begin(), rev.end());

    for(quint16 i = 0; i != size; ++i){
        if(i == 0)
            iter = QByteArray::fromStdString(_db.at(pos.toStdString()));
        else
            iter = QByteArray::fromStdString(_db.next());

        if(iter.mid(14, 4) != rev)
            continue;

        auto startKey = QByteArray::fromStdString(_db.key());
        const auto sizer = qFromLittleEndian<quint16>(iter.mid(24, 2));
        new _node(decode(), startKey, (sizer > 0) ? true : false, parent);
    }
}
void searchModel::setCatalogFile(const QDir & file, const QByteArray & pos){
    delete _root;

    _db.open(file.path().toStdString());
    _db.setIndex(4);
    _root = new _node("root", pos);

    createHeap(_root, pos);
    emit dataChanged(QModelIndex(), QModelIndex());
}
QModelIndex searchModel::index(int row, int column, const QModelIndex &parent) const{
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid()) {
        return createIndex(row, column, _root->child(row));
    }

    auto parentInfo = static_cast<_node*>(parent.internalPointer());
    return createIndex(row, column, parentInfo->child(row));
}
QModelIndex searchModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    auto childItem = static_cast<_node*>(index.internalPointer());
    auto parentItem = childItem->parent();

    if (parentItem == _root)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}
int searchModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return _root->childSize();
    }
    auto parentInfo = static_cast<const _node*>(parent.internalPointer());
    return parentInfo->childSize();
}
int searchModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}
QVariant searchModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto item = static_cast<const _node*>(index.internalPointer());
    return item->data();
}
searchModel::~searchModel(){
    delete _root;
}
Qt::ItemFlags searchModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}
bool searchModel::canFetchMore(const QModelIndex &parent) const{
    if(!parent.isValid())
        return false;

    auto parentPtr = static_cast<const _node*>(parent.internalPointer());
    return parentPtr->marker() && (parentPtr->childSize() == 0);
}
void searchModel::fetchMore(const QModelIndex &parent){
    if(!parent.isValid())
        return;

    auto parentPtr = static_cast<_node*>(parent.internalPointer());
    createHeap(parentPtr, parentPtr->key());
    emit dataChanged(QModelIndex(), QModelIndex());
}
bool searchModel::hasChildren(const QModelIndex &parent) const{
    if (parent.isValid()) {
        auto parentInfo = static_cast<const _node*>(parent.internalPointer());
        if (parentInfo->marker()) {
            return true;
        }
    }
    return QAbstractItemModel::hasChildren(parent);
}
