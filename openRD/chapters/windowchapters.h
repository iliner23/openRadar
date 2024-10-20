#ifndef WINDOWCHAPTERS_H
#define WINDOWCHAPTERS_H

#include <QtWidgets>
#include <memory>
#include "openctree.h"
#include "repertory/repertory.h"
#include "searchmodel.h"
#include "delegate.h"
#include "proxysearchmodel.h"
#include "commonfunctions.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class windowChapters;
}
QT_END_NAMESPACE

class windowChapters : public QDialog
{
    Q_OBJECT
public:
    explicit windowChapters(QWidget *parent = nullptr);
    ~windowChapters();
public slots:
    void show(QList<QMdiSubWindow*>, QMdiSubWindow*);
    void show(QList<QMdiSubWindow*>, QMdiSubWindow *, const QByteArray);
    void reject();
signals:
    void activatedBranch(const QModelIndex &, const quint32);
private slots:
    void show() {}
    void chapterTextFilter(const QString);
    void symptomTextFilter(const QString);
    void selectedItemTable(QTableWidgetItem *);
    void chapterAccept();
    void symptomReject();
    void listClicked(const QModelIndex &);
    void returnBranch();
    void sendActivatedBranch();
    void selectedItemList(const QModelIndex &);
    void setActiveRepertory(int);
private:
    void changeChapterText(const QByteArray);
    void tableRender(int);
    void showListChapter(const QByteArray);
    inline void clearModel();

    Ui::windowChapters *ui;
    QVector<QDir> _dirPaths;
    QTextCodec * _codec;
    QStackedLayout * _layout;
    searchModel * _model = nullptr;
    proxySearchModel * _filterModel;
};

#endif // WINDOWCHAPTERS_H
