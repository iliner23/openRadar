#ifndef REPERTORYWORDS_H
#define REPERTORYWORDS_H

#include <QAbstractListModel>
#include <QtWidgets>
#include <QTextCodec>
#include "openctree.h"

class repertoryWords : public QAbstractListModel
{
public:
    explicit repertoryWords(QObject *parent = nullptr);
    explicit repertoryWords(const QDir & wordFile, QObject * parent = nullptr);
    void setWordFile(const QDir & wordFile);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
private:
    QStringList _words;
    openCtree _word;
    QTextCodec * _codec = nullptr;
};

#endif // REPERTORYWORDS_H
