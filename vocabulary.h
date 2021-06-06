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
    explicit vocabulary(QWidget *parent = nullptr);
    ~vocabulary();
private:
    Ui::vocabulary *ui;
    int _pos = 0;
    QTextCodec * _codec = nullptr;
    QDir _catalog;

    QStringListModel * _model;
    QSortFilterProxyModel * _filter;

    bool _typeAdded = false;

    inline void renderingWords(openCtree && base);
signals:
    void sendChoosenChapter(QByteArray);
private slots:
    void show() {}
    void open() {}
    void changedLanguage();
    void rendering(const int);
    void filter(const QString &);
    void selectedModelItem(const QModelIndex &);
    void clearPlainText();
public slots:
    void open(QMdiSubWindow*);
    void reject();
};

#endif // VOCABULARY_H
