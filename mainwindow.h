#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <memory>
#include "openctree.h"
#include "repchose.h"
#include "repertory.h"
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
    void openRepertory(QModelIndex &, const quint16, QTextCodec *);

    void openChapters();
    void openChaptersInCurrentPos();

    void windowChanged();
    void windowActivated(QAction*);
    void setPositionInRepertory(const QModelIndex &, const qint32);
private:
    Ui::MainWindow *ui;
    RepChose * _choose = nullptr;
    windowChapters * _chapters = nullptr;
    openCtree _catalog;
    QStringList _reperts;
    QStringList _keys;
    std::shared_ptr<cache> _cache;
};
#endif // MAINWINDOW_H
