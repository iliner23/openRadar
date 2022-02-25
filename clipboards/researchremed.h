#ifndef RESEARCHREMED_H
#define RESEARCHREMED_H

#include <QtWidgets>
#include "researchremedrender.h"

namespace Ui {
class researchRemed;
}

class researchRemed : public QMainWindow
{
    Q_OBJECT
    using rci = func::remedClipboardInfo;
public:
    explicit researchRemed(QStringList clipNames, const std::shared_ptr<func::cache> &, QWidget *parent = nullptr);
    ~researchRemed();
private slots:
    void setOrientation(Qt::Orientation);
    void openStrategyMenu();
    void testOrien();
    void triggeredStrategy(QAction*);
public slots:
    void setClipboards(std::array<bool, 10>);
    void setClipboardName(QStringList);
    void setClipboardRemed(std::array<QVector<rci>, 10>);
private:
    Ui::researchRemed *ui;

    struct clipList{
        QLabel * label = nullptr;
        QPushButton * exit = nullptr;
    };

    std::array<clipList, 10> _labels;
    researchRemedRender _render;
    QStringList _clipNames;
    QGraphicsScene * _scene;
    QMenu * _strategyMenu;

    void renameLabels();
    void drawLabels(std::array<bool, 10> act);
    void drawScene();
};

#endif // RESEARCHREMED_H
