#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <memory>
#include <array>
#include "openctree/openctree.h"
#include "repertory/repchose.h"
#include "repertory/repertory.h"
#include "chapters/windowchapters.h"
#include "commonfunction/languages.h"
#include "keys/keychoose.h"
#include "clipboards/researchremed.h"
#include "clipboards/takeremed.h"

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

    void setClipboardsName();
    void addClipboardsRemed(func::remedClipboardInfo, quint8);
    void openResearchTest(QAction *act);
signals:
    void changeClipboardsName();
    void changeClipboardsRemed();
private:
    Ui::MainWindow *ui;
    RepChose * _choose = nullptr;
    KeyChoose * _keych = nullptr;
    keysRemedList * _remedList = nullptr;
    windowChapters * _chapters = nullptr;
    takeRemed * _takeRemed = nullptr;

    researchRemed * _research = nullptr;
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;

    QVector<QAction*> _pressedClipboard;
    bool _pressed = false;

    openCtree _catalog;
    QVector<QDir> _repertsPos;
    QVector<std::pair<QLocale, QLocale>> _repertsLang;
    std::shared_ptr<func::cache> _cache;
    std::shared_ptr<std::array<QVector<rci>, 10>> _clipboadrs;
    std::shared_ptr<QStringList> _clipNames;
};
#endif // MAINWINDOW_H
