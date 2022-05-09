#include "researchremed.h"
#include "ui_researchremed.h"

researchRemed::researchRemed(std::shared_ptr<QStringList> clipNames, std::shared_ptr<std::array<QVector<rci>, 10>> clipRemed, const std::shared_ptr<func::cache> & cache, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::researchRemed){
    ui->setupUi(this);
    _clipNames = clipNames;
    _clipRemed = clipRemed;
    _scene = new QGraphicsScene(this);
    _render.reset(cache);
    ui->graphicsView->setScene(_scene);

    _strategyMenu = new QMenu(ui->strategy);
    _showMenu = new QMenu(ui->show);
    _listMenu = new QMenu(ui->listWidget);

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

    {//listWidget contex menu
        QStringList labels = { "Редактировать - Свойства" };
        addActions(_listMenu, _listAction, labels, false);
        ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    }

    connect(_listMenu, &QMenu::triggered, this, &researchRemed::triggeredList);
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, &researchRemed::openListMenu);
    connect(_strategyMenu, &QMenu::triggered, this, &researchRemed::triggeredStrategy);
    connect(_showMenu, &QMenu::triggered, this, &researchRemed::triggeredShow);
}
void researchRemed::renameLabels(){
    for(auto i = 0; i != 10; ++i)
        _labels[i]->setText(_clipNames->at(i));
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

    for(qsizetype iter = 0; iter != act.size(); ++iter){
        if(!act.at(iter) || _clipRemed->at(iter).isEmpty())
            continue;

        auto widget = new QWidget;
        auto layout = new QHBoxLayout;
        _labels[iter] = new QLabel(_clipNames->at(iter));
        auto exit = new QPushButton("Закрыть клипборд");
        connect(exit, &QPushButton::clicked, this, [=](){researchRemed::closeClipboard(_labels[iter]);});

        auto item = new QListWidgetItem;

        layout->addWidget(_labels[iter]);
        layout->addWidget(exit);
        layout->setSizeConstraint(QLayout::SetFixedSize);
        widget->setLayout(layout);

        exit->setStyleSheet("background-color: gray");
        item->setBackground(Qt::gray);
        item->setSizeHint(widget->sizeHint());

        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, widget);

        widget->setAttribute(Qt::WA_DeleteOnClose);
        exit->setAttribute(Qt::WA_DeleteOnClose);
        _labels[iter]->setAttribute(Qt::WA_DeleteOnClose);

        const auto & clipData = _clipRemed->at(iter);

        for(qsizetype it = 0; it != clipData.size(); ++it){
            openCtree data(clipData.at(it).path.filePath("symptom").toStdString());
            data.at(clipData.at(it).key.toStdString(), false);
            auto name = func::renderingLabel(data, false, clipData.at(it).codec);

            if(name.indexOf('\n') == -1)
                name.append('\n');

            const QString groupStr = _clipRemed->at(iter).at(it).group %
                    ((_clipRemed->at(iter).at(it).group.isEmpty()) ? "" : ". ");

            auto item = new QListWidgetItem;
            item->setText(groupStr % name);

            QVariant var;
            var.setValue(std::make_pair(iter, it));
            item->setData(Qt::UserRole, var);

            ui->listWidget->addItem(item);
        }

        _render.setClipboardHeight(ui->listWidget->sizeHintForRow(0));
        _render.setSymptomHeight(ui->listWidget->sizeHintForRow(1));
    }

    //NOTE : only for test
    _render.setAnalysisType(researchRemedRender::showType::waffle);
}
void researchRemed::setClipboardName(){
    if(!isHidden())
        drawLabels(_render.showedClipboards());
}
void researchRemed::setClipboardRemed(){
    _render.setClipboards(*_clipRemed);

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
void researchRemed::closeClipboard(QLabel * closeButton){
    auto findIter = std::find(_labels.cbegin(), _labels.cend(), closeButton);
    auto showClip = _render.showedClipboards();

    showClip.at(findIter - _labels.cbegin()) = false;
    _render.setShowedClipboards(showClip);

    if(!isHidden())
        setClipboards(showClip);
}
void researchRemed::resizeEvent(QResizeEvent * event){
    if(ui->graphicsView->isVisible() && ui->listWidget->count() != 0)
        drawScene();

    event->ignore();
}
void researchRemed::openListMenu(const QPoint & point){
    auto index = ui->listWidget->itemAt(point);//TODO: test functionality
    auto globalPos = ui->listWidget->mapToGlobal(point);

    if(index != nullptr && ui->listWidget->itemWidget(index) == nullptr)
        _listMenu->exec(globalPos);
}
void researchRemed::triggeredList(QAction * action){
    const auto act = _listAction.indexOf(action);//TODO: test functionality

    if(act != 0)
        return;

    auto item = ui->listWidget->currentItem();

    if(item != nullptr && ui->listWidget->itemWidget(item) == nullptr){
        auto setRmd = new setRemedy(_clipRemed,
            item->data(Qt::UserRole).value<std::pair<qsizetype, qsizetype>>(), this);

        setRmd->setAttribute(Qt::WA_DeleteOnClose);
        connect(setRmd, &setRemedy::clipboardRemedChanged, this, &researchRemed::setClipboardRemed);
        setRmd->exec();
    }
}
researchRemed::~researchRemed(){
    delete ui;
}
