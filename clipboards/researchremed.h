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
    void show(std::array<bool, 10>);
    ~researchRemed();
public slots:

    //void setClipboards(std::array<bool, 10>);
    void setClipboardName();
    void setClipboardRemed();
private:
    Ui::researchRemed *ui;

    std::array<QLabel*, 10> _labels;
    researchRemedRender _render;
    std::shared_ptr<QStringList> _clipNames;
    QGraphicsScene * _scene_header, * _scene_remedies, * _scene_counter;

    QMenu * _strategyMenu, * _showMenu, * _listMenu;
    QList<QAction*> _sympthomAndAnalis;
    QList<QAction*> _strategy;
    QList<QAction*> _listAction;
    QAction * _intensity;
    QList<QAction*> _orientation;
    quint8 _hide = 2;//0 - hide view, 1 - hide list, 2 - show all
    qint32 _oldHeight = 0, _beginPos = 0;

    std::shared_ptr<std::array<QVector<rci>, 10>> _clipRemed;

    void renameLabels();
    void drawLabels(std::array<bool, 10> act);
    void drawHeader();
    void drawRemedies(bool partlyRendering = false);
    void resizeEvent(QResizeEvent * event) override;
    inline qreal horizontalSize() const;
    inline void hideAndShowSliders();
private slots:

    void setOrientation(Qt::Orientation);
    void closeClipboard(QLabel*);

    void openStrategyMenu();
    void openShowMenu();
    void openListMenu(const QPoint&);

    void triggeredStrategy(QAction*);
    void triggeredShow(QAction*);
    void triggeredList(QAction*);

    void splitterMoved(int, int);

    void hScrollBarMoved(int);
    void vScrollBarMoved(int);
};

#endif // RESEARCHREMED_H
