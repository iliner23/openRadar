#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <memory>
#include "openctree.h"
#include "repchose.h"
#include "repertory.h"
#include "windowchapters.h"
#include "languages.h"
#include "keychoose.h"
#include "researchremed.h"
#include "takeremed.h"

#define _TEST_

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    using rci = func::remedClipboardInfo;
private slots:
    void on_action1_triggered();
    void openKeysChooseWindow();
    void openRepertory(QModelIndex &, const quint16,
                       std::pair<QLocale, QLocale>);

    void openChapters();
    void openChaptersInCurrentPos();

    void windowChanged();
    void windowActivated(QAction*);
    void setPositionInRepertory(const QModelIndex &, const qint32);

    void openVocabulary();
    void openTakeRemed();

    void setClipboardsName(QStringList name);
    void addClipboardsRemed(func::remedClipboardInfo, quint8);
#ifdef _TEST_
    void openResearchTest(QAction *act);
#endif
signals:
    void changeClipboardsName(QStringList);
    void changeClipboardsRemed(std::array<QVector<rci>, 10>);
private:
    Ui::MainWindow *ui;
    RepChose * _choose = nullptr;
    KeyChoose * _keych = nullptr;
    keysRemedList * _remedList = nullptr;
    windowChapters * _chapters = nullptr;
    takeRemed * _takeRemed = nullptr;

#ifdef _TEST_
    researchRemed * _research = nullptr;
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;

    QVector<QAction*> _pressedClipboard;
    bool _pressed = false;
#endif
    openCtree _catalog;
    QVector<QDir> _repertsPos;
    QVector<std::pair<QLocale, QLocale>> _repertsLang;
    std::shared_ptr<func::cache> _cache;
    std::array<QVector<rci>, 10> _clipboadrs;

    QStringList _clipNames = {
        "Клипборд 1",
        "Клипборд 2",
        "Клипборд 3",
        "Клипборд 4",
        "Клипборд 5",
        "Клипборд 6",
        "Клипборд 7",
        "Клипборд 8",
        "Клипборд 9",
        "Клипборд 10",
    };
};
#endif // MAINWINDOW_H
