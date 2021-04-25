#ifndef WINDOWCHAPTERS_H
#define WINDOWCHAPTERS_H

#include <QDialog>
#include <openctree.h>
#include <QMdiSubWindow>
#include <repertory.h>
#include <vector>
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
private:
    void windowChanged(int);
    void showListChapter(const QByteArray);
private slots:
    void textFilter(const QString &);
    void selectedItem(QTableWidgetItem *);
    void accept_1();
    void reject_2();

private:
    Ui::windowChapters *ui;
    std::vector<QDir> _dirPaths;
    QTextCodec * _codec;
    QStackedLayout * _layout;
    QByteArray _root;
    std::unique_ptr<searchModel> _model;
};

#endif // WINDOWCHAPTERS_H
