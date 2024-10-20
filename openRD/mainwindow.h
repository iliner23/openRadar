#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <memory>
#include <array>
#include "openctree.h"
#include "repertory/repchose.h"
#include "repertory/repertory.h"
#include "chapters/windowchapters.h"
#include "languages.h"
#include "widgetaction.h"
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
    void openRepertoryWindow();
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

    //void setClipboardsName();
    void addClipboardsRemed(func::remedClipboardInfo, quint8);
    void clipboadrsDataChanged();
    //void openResearchTest(QAction *act);
    void openResearchTest(qint8);

    void addClipboardsRemedIntense1();
    void addClipboardsRemedIntense2();
    void addClipboardsRemedIntense3();
    void addClipboardsRemedIntense4();
    void dragAndDropAddRemed(QByteArray, qint8);
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
    widgetAction * _clipboardsActions[10];

    researchRemed * _research = nullptr;
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    void setDefaultRemedClipboardInfo(func::remedClipboardInfo &);
    inline void changeIcon(qint8 pos, bool opened);

    //QVector<QAction*> _pressedClipboard;
    QVector<qint8> _pressedClipboard;
    bool _pressed = false;

    openCtree _catalog;
    QVector<QDir> _repertsPos;
    QVector<std::pair<QLocale, QLocale>> _repertsLang;
    std::shared_ptr<func::cache> _cache;
    std::shared_ptr<std::array<QVector<rci>, 10>> _clipboadrs;
    std::shared_ptr<QStringList> _clipNames;

    //defaults
    qint8 _defaultClipboard = 0;
};
#endif // MAINWINDOW_H
