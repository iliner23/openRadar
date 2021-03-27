#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "openctree.h"
#include "repchose.h"
#include "repertory.h"
#include <QDir>
#include <QStringListModel>
#include <QListWidgetItem>
#include <QMdiSubWindow>
#include "cache.h"
#include "windowchapters.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action1_triggered();
    void openRepertory(QListWidgetItem *, const quint16);
    void openChapters();
    void windowChanged();
    void windowActivated(QAction*);
private:
    Ui::MainWindow *ui;
    RepChose * _choose = nullptr;
    windowChapters * _chapters = nullptr;
    openCtree _catalog;
    QStringList _reperts;
    QStringList _keys;
    cache _cache;
};
#endif // MAINWINDOW_H
