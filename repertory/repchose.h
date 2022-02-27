#ifndef REPCHOSE_H
#define REPCHOSE_H

#include <QtWidgets>
#include <QTextCodec>
#include "openctree/openctree.h"
#include "commonfunction/languages.h"

namespace Ui {
class RepChose;
}

class RepChose : public QDialog
{
    Q_OBJECT

public:
    explicit RepChose(const QStringList, const QVector<QDir> reppos,
                      QVector<std::pair<QLocale, QLocale>>,
                      QWidget *parent = nullptr);
    ~RepChose();

private:
    Ui::RepChose *ui;
    QStringList _rLevels;
    quint16 _repLevel = -1;
    QSortFilterProxyModel * _proxyModel;
    QStringListModel * _model;
    QTextCodec * _codec = nullptr;
    QVector<std::pair<QLocale, QLocale>> _lang;
public slots:
    void accept();
private slots:
    void finding(const QString);
    void activateLevel(const QModelIndex &);
    void showLevels();
signals:
    void chooseRep(QModelIndex &, quint16,
                   std::pair<QLocale, QLocale>);
};

#endif // REPCHOSE_H
