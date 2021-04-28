#ifndef WINDOWCHAPTERS_H
#define WINDOWCHAPTERS_H

#include <QDialog>
#include <openctree.h>
#include <QMdiSubWindow>
#include <repertory.h>
#include <QString>
#include <QStackedLayout>
#include "delegate.h"
#include <QTextCodec>
#include <QPushButton>
#include <QTableWidgetItem>
#include <memory>
#include "searchmodel.h"

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
    void selectedItem(QTableWidgetItem *);
    void accept_1();
    void reject_2();
    void listClicked(const QModelIndex &);
    void returnBranch();
    void sendActivatedBranch();
signals:
    void activatedBranch(const QModelIndex &, const quint32);

private:
    void changeChapterText(const QByteArray &);

    Ui::windowChapters *ui;
    QVector<QDir> _dirPaths;
    QTextCodec * _codec;
    QStackedLayout * _layout;
    std::unique_ptr<searchModel> _model;
};

#endif // WINDOWCHAPTERS_H
