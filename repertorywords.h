#ifndef REPERTORYWORDS_H
#define REPERTORYWORDS_H

#include <QAbstractListModel>
#include <QtWidgets>
#include "openctree.h"

class repertoryWords : public QAbstractListModel
{
public:
    explicit repertoryWords(QObject *parent = nullptr);
    explicit repertoryWords(const QDir & wordFile, QObject * parent = nullptr);
    void setWordFile(const QDir & wordFile);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
};

#endif // REPERTORYWORDS_H
