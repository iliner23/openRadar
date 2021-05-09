#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <QAbstractItemModel>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QTextCodec>
#include <QtEndian>
#include <QVector>
#include <QDebug>
#include "openctree.h"

class searchModel : public QAbstractItemModel
{
public:
    class _node{
    private:
        friend searchModel;
        explicit _node(const QString & data, const QByteArray & key = "", bool marker = false, _node * parent = nullptr);

        QString _data;
        std::vector<_node*> _children;
        _node * _parent = nullptr;
        quint16 _row = 0;
        QByteArray _key;
        bool _marker;
    public:
        ~_node();
        auto * parent() const noexcept { return _parent; }
        auto key() const noexcept { return _key; }
        auto row() const noexcept { return _row; }
        auto data() const noexcept { return _data; }
        auto childSize() const noexcept { return _children.size(); }
        _node * child(int pos) const noexcept;
        bool marker() const noexcept { return _marker;};
    };

    searchModel(QObject * parent = nullptr): QAbstractItemModel(parent) { _root = new _node("root"); }
    searchModel(const QDir &, const QByteArray &, QObject * parent = nullptr);
    ~searchModel();
    void setCatalogFile(const QDir &, const QByteArray &);
    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;
    bool hasChildren(const QModelIndex &parent) const override;
private:
    _node * _root = nullptr;
    openCtree _db;

    void createHeap(_node *, QByteArray);
};

#endif // SEARCHMODEL_H
