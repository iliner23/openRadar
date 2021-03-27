#ifndef WINDOWCHAPTERS_H
#define WINDOWCHAPTERS_H

#include <QDialog>
#include <openctree.h>
#include <QMdiSubWindow>
#include <repertory.h>
#include <vector>
#include <QString>
#include "delegate.h"
#include <QTextCodec>

QT_BEGIN_NAMESPACE
namespace Ui {
class windowChapters;
}
QT_END_NAMESPACE

class windowChapters : public QDialog
{
public:
    explicit windowChapters(QWidget *parent = nullptr);
    ~windowChapters();
    void getWindows(QList<QMdiSubWindow*>, QMdiSubWindow*);
    void show();
private:
    void windowChanged(int);

private:
    Ui::windowChapters *ui;
    std::vector<QDir> _dirPaths;
    QTextCodec * _codec;
};

#endif // WINDOWCHAPTERS_H
