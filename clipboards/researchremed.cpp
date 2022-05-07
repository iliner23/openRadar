#include "researchremed.h"
#include "ui_researchremed.h"

researchRemed::researchRemed(QStringList clipNames, const std::shared_ptr<func::cache> & cache, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::researchRemed){
    ui->setupUi(this);
    _clipNames = clipNames;
    _scene = new QGraphicsScene(this);
    _render.reset(cache);
    ui->graphicsView->setScene(_scene);

    _strategyMenu = new QMenu(ui->strategy);
    _showMenu = new QMenu(ui->show);

    auto addActions = [](auto * menu, auto & actions, const auto labels, bool checkable = true){
        for(const auto & item : labels){
            auto action = new QAction(item, menu);
            if(checkable)
                action->setCheckable(true);
            actions.push_back(action);
        }

        menu->addActions(actions);
    };

    {//strategy
        const QStringList labels = { "Сумма симптомов, сорт: степень", "Сумма степеней, сорт: симптом",
                                     "Сумма симптомов и степеней", "Сумма симптомов", "Сумма степеней" };

        addActions(_strategyMenu, _strategy, labels);
        _strategy.at(2)->setChecked(true);
        _strategyMenu->addSeparator();

        _intensity = new QAction("Учитывать интенсивность", _strategyMenu);
        _intensity->setCheckable(true);
        _intensity->setChecked(true);
        _strategyMenu->addAction(_intensity);
    }
    {//show
        QStringList labels = { "Только симптомы", "Только анализ", "Симптомы и анализ" };

        addActions(_showMenu, _sympthomAndAnalis, labels, false);
        _showMenu->addSeparator();

        labels = QStringList{ "Рядом", "Над/Под" };
        addActions(_showMenu, _orientation, labels);
        _orientation.at(0)->setChecked(true);
    }

    connect(_strategyMenu, &QMenu::triggered, this, &researchRemed::triggeredStrategy);
    connect(_showMenu, &QMenu::triggered, this, &researchRemed::triggeredShow);
}
void researchRemed::renameLabels(){
    for(auto i = 0; i != 10; ++i)
        _labels[i].label->setText(_clipNames.at(i));
}
void researchRemed::drawScene(){
    auto var = _render.render(ui->graphicsView->size());
    _scene->clear();
    _scene->setSceneRect(0, 0, var->boundingRect().width(), var->boundingRect().height());
    _scene->addItem(var);
    ui->spacefiller->setMinimumHeight(_render.labelHeight());
}
void researchRemed::setClipboards(std::array<bool, 10> act){
    drawLabels(act);
    setOrientation(_render.orientation());
    drawScene();
}
void researchRemed::drawLabels(std::array<bool, 10> act){
    ui->listWidget->clear();
    _render.setShowedClipboards(act);
    const auto clipboards = _render.clipboards();

    for(auto iter = 0; iter != act.size(); ++iter){
        if(!act.at(iter) || clipboards.at(iter).isEmpty())
            continue;

        auto widget = new QWidget;
        auto layout = new QHBoxLayout;
        _labels[iter].label = new QLabel(_clipNames.at(iter));
        _labels[iter].exit = new QPushButton("Закрыть клипборд");
        auto item = new QListWidgetItem;

        layout->addWidget(_labels[iter].label);
        layout->addWidget(_labels[iter].exit);
        layout->setSizeConstraint(QLayout::SetFixedSize);
        widget->setLayout(layout);

        _labels[iter].exit->setStyleSheet("background-color: gray");
        item->setBackground(Qt::gray);
        item->setSizeHint(widget->sizeHint());
        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, widget);

        for(auto & it : clipboards.at(iter)){
            openCtree data(it.path.filePath("symptom").toStdString());
            data.at(it.key.toStdString(), false);
            auto name = func::renderingLabel(data, false, it.codec);

            if(name.indexOf('\n') == -1)
                name.append('\n');

            ui->listWidget->addItem(name);
        }

        _render.setClipboardHeight(ui->listWidget->sizeHintForRow(0));
        _render.setSymptomHeight(ui->listWidget->sizeHintForRow(1));
    }

    //NOTE : only for test
    _render.setAnalysisType(researchRemedRender::showType::waffle);
}
void researchRemed::setClipboardName(QStringList name){
    _clipNames = name;

    if(!isHidden())
        drawLabels(_render.showedClipboards());
}
void researchRemed::setClipboardRemed(std::array<QVector<rci>, 10> array){
    _render.setClipboards(array);

    if(!isHidden()){
        drawLabels(_render.showedClipboards());
        drawScene();
    }
}
void researchRemed::setOrientation(Qt::Orientation orien){
    _render.setOrientation(orien);
    ui->spacefiller->setHidden(orien == Qt::Horizontal);
    ui->splitter->setOrientation((orien == Qt::Horizontal)
                                 ? Qt::Vertical : Qt::Horizontal);

    const auto list = (_hide == 1) ? 0 :
                ((orien == Qt::Vertical) ? ui->listWidget->size().height() :
                                           ui->listWidget->size().width());
    const auto view = (_hide == 0) ? 0 :
                ((orien == Qt::Vertical) ? ui->graphicsView->size().height() :
                                           ui->graphicsView->size().width());

    ui->splitter->setSizes({ list, view });
    QApplication::processEvents();
}
void researchRemed::openStrategyMenu(){
    auto pos = ui->strategy->pos();
    pos.setY(pos.y() + ui->strategy->height());
    _strategyMenu->popup(QPoint(mapToGlobal(pos)));
}
void researchRemed::openShowMenu(){
    auto pos = ui->show->pos();
    pos.setY(pos.y() + ui->strategy->height());
    _showMenu->popup(QPoint(mapToGlobal(pos)));
}
void researchRemed::triggeredStrategy(QAction * action){
    auto pos = _strategy.indexOf(action);

    if(pos != -1){
        researchRemedRender::strategy strategy[] =
                { researchRemedRender::strategy::sumRemediesBySortDegrees,
                  researchRemedRender::strategy::sumDegreesBySortRemedies,
                  researchRemedRender::strategy::sumRemediesAndDegrees,
                  researchRemedRender::strategy::sumRemedies,
                  researchRemedRender::strategy::sumDegrees };

        for(auto & it : _strategy)
            it->setChecked(false);

        action->setChecked(true);
        _render.setStrategyType(strategy[pos]);
    }
    else if(action == _intensity)
        _render.setConsideIntencity(_intensity->isChecked());
    else
        return;

    if(!isHidden())
        drawScene();
}
void researchRemed::triggeredShow(QAction * action){
    auto checking = [&](auto actions){
        for(auto it = 0; it != actions.size(); ++it)
            actions.at(it)->setChecked(false);

        action->setChecked(true);
    };

    auto pos = _sympthomAndAnalis.indexOf(action);

    if(pos != -1){
        checking(_sympthomAndAnalis);
        _hide = pos;
        setOrientation(_render.orientation());

        if(ui->graphicsView->isVisible())
            drawScene();

        return;
    }

    pos = _orientation.indexOf(action);

    if(pos != -1){
        checking(_orientation);
        setOrientation((pos == 0) ? Qt::Vertical : Qt::Horizontal);

        if(ui->graphicsView->isVisible())
            drawScene();

        return;
    }
}
void researchRemed::resizeEvent(QResizeEvent * event){
    drawScene();
    event->ignore();
}
researchRemed::~researchRemed(){
    delete ui;
}
