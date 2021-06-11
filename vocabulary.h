#ifndef VOCABULARY_H
#define VOCABULARY_H

#include <QtWidgets>
#include <QTextCodec>
#include "openctree.h"
#include "repertory.h"

namespace Ui {
class vocabulary;
}

class vocabulary : public QDialog
{
    Q_OBJECT

public:
    explicit vocabulary(const QDir & system, QWidget *parent = nullptr);
    ~vocabulary();
private:
    Ui::vocabulary *ui;
    QTextCodec * _codec = nullptr;
    QDir _catalog, _system;

    QStringListModel * _model;
    QSortFilterProxyModel * _filter;

    bool _typeAdded = false;

    inline void renderingWords(openCtree);
    inline void renderingRoots(openCtree);
    inline void threadsLaunch(openCtree &, std::function<QStringList (openCtree, const int, const int)>);
signals:
    void sendChoosenChapter(QByteArray);
private slots:
    void show() {}
    void open() {}
    void changedLanguage();
    void rendering(const int);
    void filter(const QString &);
    void selectedModelItem(const QModelIndex &);
    void changedPlainText();
public slots:
    void open(QMdiSubWindow*);
    void reject();
};

#endif // VOCABULARY_H
