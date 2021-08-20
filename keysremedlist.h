#ifndef KEYSREMEDLIST_H
#define KEYSREMEDLIST_H

#include <QtWidgets>
#include <QTextCodec>
#include <QtConcurrent/QtConcurrent>
#include "openctree.h"
#include "languages.h"
#include "keyreader.h"

namespace Ui {
class keysRemedList;
}

class keysRemedList : public QDialog
{
    Q_OBJECT

public:
    explicit keysRemedList(const QVector<QDir> keysFile,
                           QStringList keysText, QWidget *parent = nullptr);
    ~keysRemedList();

public slots:
    void changeTable(int);
private slots:
    void renderingTable(int);
    void fullFilter(QString);
    void shortFilter(QString);
    void openReader(const QModelIndex &);
private:
    Ui::keysRemedList *ui;
    QVector<QDir> _keysFile;
    QTextCodec * _codec;
    QStringList _keysText;

    QStandardItemModel * _model;
    QSortFilterProxyModel * _proxy;
};

#endif // KEYSREMEDLIST_H
