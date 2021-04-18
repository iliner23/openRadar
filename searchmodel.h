#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <QAbstractListModel>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QTextCodec>
#include <QtEndian>
#include "openctree.h"

class searchModel : public QAbstractListModel
{
public:
    searchModel(QObject * parent = 0);
    void setCatalogFile(const QDir &, QByteArray);
    QDir catalogFile() const noexcept;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
private:
    openCtree _db;
    QByteArray _fpos;
    QDir _file;
    QStringList _data;
};

#endif // SEARCHMODEL_H
