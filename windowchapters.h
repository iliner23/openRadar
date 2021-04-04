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
    void show();
private:
    void windowChanged(int);
private slots:
    void textFilter(const QString &);
    void selectedItem(QTableWidgetItem *);

private:
    Ui::windowChapters *ui;
    std::vector<QDir> _dirPaths;
    QTextCodec * _codec;
};

#endif // WINDOWCHAPTERS_H
