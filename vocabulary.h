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
    QVector<std::pair<QDir, QTextCodec *>> _catalogs;
    bool _language = false;

    QStringListModel * _word;
    QSortFilterProxyModel * _filter;

    inline void renderingWords();
signals:
    void sendChoosenChapter(QByteArray);
private slots:
    void show() {}
    void open() {}
    void changedLanguage(const int);
    void rendering(const int);
    void filter(const QString &);
public slots:
    void open(QList<QMdiSubWindow*>, QMdiSubWindow*);
};

#endif // VOCABULARY_H
