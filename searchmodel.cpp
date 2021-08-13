#include "searchmodel.h"
searchModel::_node::_node(const QString data, const QByteArray key, bool marker, _node * parent){
    _data = data;
    _parent = parent;
    _marker = marker;
    _key = key;
    _row = 0;
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

searchModel::searchModel(const QDir filename, const QByteArray data, QTextCodec * codec, QObject * object): QAbstractItemModel(object){
    setCatalogFile(filename, data, codec);
}
void searchModel::createHeap(_node * parent, QByteArray pos){
    const auto startStr = QByteArray::fromStdString(_db.at(pos.toStdString()));

    auto decode = [&](const auto & iter){
        auto return_size = [](const auto & value, const auto & _size){
            return (value == -1) ? _size : value;
        };

        const auto children = qFromLittleEndian<quint16>(iter.constData() + 24);
        const auto firstIt = iter.indexOf('\0', _db.serviceDataLenght());
        auto secondIt = iter.indexOf('\0', firstIt + 1);
        const auto localize = _codec->toUnicode(iter.mid(firstIt + 1, secondIt - firstIt - 1));
        const auto original = _codec->toUnicode(iter.mid(_db.serviceDataLenght(), firstIt - _db.serviceDataLenght()));
        QString synonym[2];

        quint8 type = 4;

        for(auto i = 0; i != 4; ++i){//get synonyms
            quint8 cnt = 0;

            for(auto it = 0; it != type + 1; ++it){
                if(secondIt == iter.size()
                        || iter.at(secondIt) != '\0')
                    break;

                ++cnt;
                ++secondIt;
            }

            if(cnt > type)
                break;

            type = type - cnt;

            const auto tmpLinkIter = return_size
                    (iter.indexOf('\0', secondIt), iter.size());

            auto synLinkText =
                    _codec->toUnicode(iter.mid
                        (secondIt, tmpLinkIter - secondIt));
            secondIt = tmpLinkIter;

            if(synLinkText.size() > 1 && synLinkText.left(2) == "(="){
                if(type == 2)
                    synonym[1] = synLinkText;
                else if(type == 3){
                    if(!localize.isEmpty())
                        synonym[0] = synLinkText;
                    else
                        synonym[1] = synLinkText;
                }
            }

            if(type == 0)
                break;
        }

        QString tmpStr = ((children > 0) ? "+" : QString()) + original;
        tmpStr += ((localize.isEmpty()) ? ' ' + synonym[1] :
                   ' ' + synonym[0] + "\n" + localize + ' ' + synonym[1]);
        return tmpStr;
    };

    auto increase = [](auto & array, const auto startPos){
        for(auto i = startPos; i != 5; --i){
            if(std::numeric_limits<uchar>::max() != (uchar) array.at(i)){
                array[i] = ((uchar) array.at(i)) + 1;
                break;
            }
            else{
                if(i == 6)
                    Q_ASSERT("error increment key");

                array[i] = (uchar) 0;
            }
        }
    };

    if(startStr.left(6) == QByteArray(6, '\0')){
        _db.close();
        return;
    }

    const auto size = qFromLittleEndian<quint16>(startStr.constData() + 24);
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

    auto threadPred = [&](openCtree data, const quint16 from, const quint16 until){
        QVector<_node*> children;
        children.reserve(until - from);

        for(int i = from; i != until; ++i){
            QByteArray iter;

            if(i == from){
                data.at(pos.toStdString(), false);
                const auto posData = data.currentPosition() + from;
                iter = QByteArray::fromStdString(data.at(posData));
            }
            else
                iter = QByteArray::fromStdString(data.next());

            auto startKey = QByteArray::fromStdString(data.key());

            if(iter.mid(14, 4) != rev)
                continue;

            const auto sizer = qFromLittleEndian<quint16>(iter.constData() + 24);
            children.push_back(new _node(decode(iter), startKey, (sizer > 0) ? true : false, parent));
        }

        return children;
    };

    QVector<QFuture<QVector<_node*>>> threads;

    const auto maxThreads = (size > QThread::idealThreadCount()) ?
                QThread::idealThreadCount() : size;
    const auto del = size / maxThreads;

    for(auto i = 0; i != maxThreads; ++i){
        threads.append(QtConcurrent::run(threadPred, _db, del * i ,
            (i == maxThreads - 1) ? size : del * (i + 1)));
    }

    for(auto & it : threads)
        parent->_children.append(it.result());

    for(auto i = 0; i != parent->_children.size(); ++i)
        parent->_children[i]->_row = i;
}
void searchModel::setCatalogFile(const QDir file, const QByteArray pos, QTextCodec * codec){
    beginResetModel();
    delete _root;

    _db.open(file.path().toStdString());
    _db.setIndex(4);

    _root = new _node("root", pos);

    if(codec == nullptr)
        _codec = QTextCodec::codecForName("system");
    else
        _codec = codec;

    createHeap(_root, pos);
    endResetModel();
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
    beginInsertRows(parent, 0, (int) parentPtr->childSize() - 1);
    endInsertRows();
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
void searchModel::setTextCodec(QTextCodec * codec){
    if(codec == nullptr)
        return;

    _codec = codec;
}
QModelIndex searchModel::keyToIndex(const QByteArray key, const QModelIndex & parent){
    _node * ptrData = nullptr;

    if(parent.isValid())
        ptrData = static_cast<_node*>(parent.internalPointer());
    else
        ptrData = _root;

    if(ptrData == nullptr)
        return QModelIndex();

    if(canFetchMore(parent))
        fetchMore(parent);

    for(uint64_t i = 0; i != ptrData->childSize(); ++i){
        if(ptrData->child(i)->key().right(6) == key)
            return index(i, 0, parent);
    }

    return QModelIndex();
}
