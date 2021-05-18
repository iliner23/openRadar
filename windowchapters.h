#ifndef WINDOWCHAPTERS_H
#define WINDOWCHAPTERS_H

#include <QtWidgets>
#include <memory>
#include "openctree.h"
#include "repertory.h"
#include "searchmodel.h"
#include "delegate.h"
#include "abstractengine.h"
#include "proxysearchmodel.h"

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
    void getWindows(QList<QMdiSubWindow*>, QMdiSubWindow*);
public slots:
    void show(bool chapter = true);
    void reject();
private:
    void windowChanged(int);
    void showListChapter(const QByteArray);
    inline void clearModel();
private slots:
    void textFilter(const QString &);
    void textFilter_2(const QString &);
    void selectedItemTable(QTableWidgetItem *);
    void accept_1();
    void reject_2();
    void listClicked(const QModelIndex &);
    void returnBranch();
    void sendActivatedBranch();
    void selectedItemList(const QModelIndex &);
signals:
    void activatedBranch(const QModelIndex &, const quint32);

private:
    void changeChapterText(const QByteArray &);

    Ui::windowChapters *ui;
    QVector<QDir> _dirPaths;
    QTextCodec * _codec;
    QStackedLayout * _layout;
    std::unique_ptr<searchModel> _model;
    proxySearchModel * _filterModel;
};

#endif // WINDOWCHAPTERS_H
