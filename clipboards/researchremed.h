#ifndef RESEARCHREMED_H
#define RESEARCHREMED_H

#include <QtWidgets>
#include <memory>
#include "researchremedrender.h"
#include "setremedy.h"

namespace Ui {
class researchRemed;
}

class researchRemed : public QMainWindow
{
    Q_OBJECT
    using rci = func::remedClipboardInfo;
public:
    explicit researchRemed(std::shared_ptr<QStringList> clipNames, std::shared_ptr<std::array<QVector<rci>, 10>> clipRemed,
                           const std::shared_ptr<func::cache> &, QWidget *parent = nullptr);
    ~researchRemed();
public slots:
    void setClipboards(std::array<bool, 10>);
    void setClipboardName();
    void setClipboardRemed();
private:
    Ui::researchRemed *ui;

    std::array<QLabel*, 10> _labels;
    researchRemedRender _render;
    std::shared_ptr<QStringList> _clipNames;
    QGraphicsScene * _scene;

    QMenu * _strategyMenu, * _showMenu, * _listMenu;
    QList<QAction*> _sympthomAndAnalis;
    QList<QAction*> _strategy;
    QList<QAction*> _listAction;
    QAction * _intensity;
    QList<QAction*> _orientation;
    quint8 _hide = 2;//0 - hide list, 1 - hide view, 2 - show all

    std::shared_ptr<std::array<QVector<rci>, 10>> _clipRemed;

    void renameLabels();
    void drawLabels(std::array<bool, 10> act);
    void drawScene();
    void resizeEvent(QResizeEvent * event) override;
private slots:
    void setOrientation(Qt::Orientation);
    void closeClipboard(QLabel*);

    void openStrategyMenu();
    void openShowMenu();
    void openListMenu(const QPoint&);

    void triggeredStrategy(QAction*);
    void triggeredShow(QAction*);
    void triggeredList(QAction*);
};

#endif // RESEARCHREMED_H
