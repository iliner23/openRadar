#include "researchremed.h"
#include "ui_researchremed.h"

researchRemed::researchRemed(std::shared_ptr<QStringList> clipNames, std::shared_ptr<std::array<QVector<rci>, 10>> clipRemed, const std::shared_ptr<func::cache> & cache, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::researchRemed){
    ui->setupUi(this);
    _clipNames = clipNames;
    _clipRemed = clipRemed;

    _scene_header = new QGraphicsScene(this);
    _scene_remedies = new QGraphicsScene(this);
    _scene_counter = new QGraphicsScene(this);

    _render.setCache(cache);
    ui->graphicsView_header->setScene(_scene_header);
    ui->graphicsView_remedies->setScene(_scene_remedies);
    ui->graphicsView_count->setScene(_scene_counter);

    _scene_header->setItemIndexMethod(QGraphicsScene::NoIndex);
    _scene_remedies->setItemIndexMethod(QGraphicsScene::NoIndex);

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
        _strategy.at(3)->setChecked(true);
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
        _listMenu->addAction(ui->deleteSymptom);
        _listMenu->addSeparator();
        _listMenu->addAction(ui->property);
        ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    }

    QPalette pal = QPalette();
    pal.setColor(QPalette::Window, Qt::white);

    ui->widgetFiller->setAutoFillBackground(true);
    ui->widgetFiller->setPalette(pal);

    ui->horizontalScrollBar->hide();

    connect(_strategyMenu, &QMenu::triggered, this, &researchRemed::triggeredStrategy);
    connect(_showMenu, &QMenu::triggered, this, &researchRemed::triggeredShow);
}
void researchRemed::renameLabels(){
    for(auto i = 0; i != 10; ++i)
        _labels[i]->setText(_clipNames->at(i));
}
void researchRemed::drawHeader(){
    _scene_header->clear();

    auto labels  = _render.renderLabels();
    const qreal height = labels->boundingRect().height();

    ui->graphicsView_header->setFixedHeight(height);
    _scene_header->addItem(labels);

    ui->spacefiller->setMinimumHeight(height);
}
inline void researchRemed::hideAndShowSliders(){
    const auto size = ui->graphicsView_remedies->maximumViewportSize();
    const auto data = _render.clipboardAndRemediesNumbers();
    const auto viewportSize = std::roundf(size.width() / horizontalSize());

    if(data.first <= viewportSize)
        ui->horizontalScrollBar->hide();
    else{
        ui->horizontalScrollBar->show();
        ui->horizontalScrollBar->setRange(0, data.first - 1);
    }

    //TODO : only for test
    if(ui->splitter->orientation() == Qt::Vertical){
        qsizetype counter = 0;

        for(auto i : data.second){
            if(!i)
                ++counter;
        }

        ui->verticalScrollBar->setRange(0, counter - 1);
    }
    else
        ui->verticalScrollBar->setRange(0, data.second.size() - 1);

    QApplication::processEvents();
}
void researchRemed::drawRemedies(bool partlyRendering){
    _scene_remedies->clear();

    auto size = ui->graphicsView_remedies->maximumViewportSize();
    size.setHeight(size.height() + ui->horizontalScrollBar->height());
    auto var = _render.renderRemedies(size, partlyRendering);

    if(var == nullptr)
        return;

    _scene_remedies->setSceneRect(0, 0, var->boundingRect().width(), var->boundingRect().height());
    _scene_remedies->addItem(var);

    hideAndShowSliders();

    if(ui->splitter->orientation() == Qt::Vertical && !partlyRendering){
        auto counter = _render.symptomsHCounter();
        const auto rect = counter->boundingRect();

        _scene_counter->clear();
        _scene_counter->addItem(counter);

        ui->graphicsView_count->setFixedWidth(rect.width() * 2);
        ui->widgetFiller->setFixedWidth(rect.width() * 2);
        _scene_counter->setSceneRect(0, 0, 1, 1);
    }
}
void researchRemed::show(std::array<bool, 10> act){
    drawLabels(act);
    setOrientation(_render.orientation());

    drawHeader();
    QWidget::show();
    drawRemedies();
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

    if(!ui->listWidget->count()){
        ui->verticalScrollBar->setMaximum(0);
        ui->verticalScrollBar->setEnabled(false);
    }
    else
        ui->verticalScrollBar->setEnabled(true);

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
        drawHeader();
        drawRemedies();
    }
}
void researchRemed::setOrientation(Qt::Orientation orien){
    _render.setOrientation(orien);
    ui->spacefiller->setHidden(orien == Qt::Horizontal);
    ui->splitter->setOrientation((orien == Qt::Horizontal)
                                 ? Qt::Vertical : Qt::Horizontal);

    int maxSplitterPos = 0;
    ui->splitter->getRange(1, nullptr, &maxSplitterPos);
    maxSplitterPos /= 2;

    const auto list = (_hide == 1) ? 0 : maxSplitterPos;
    const auto view = (_hide == 0) ? 0 : maxSplitterPos;

    if(orien == Qt::Horizontal){
        ui->graphicsView_count->show();
        ui->widgetFiller->show();
    }
    else{
        ui->graphicsView_count->hide();
        ui->widgetFiller->hide();
    }

    ui->splitter->setSizes({ list, view });
    ui->verticalScrollBar->setValue(0);
    ui->graphicsView_count->setSceneRect(0, 0, 1, 1);

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

    if(!isHidden()){
        drawHeader();
        QApplication::processEvents();
        drawRemedies();
    }
}
void researchRemed::triggeredShow(QAction * action){
    auto checking = [&](auto actions){
        for(auto it = 0; it != actions.size(); ++it)
            actions.at(it)->setChecked(false);

        action->setChecked(true);
    };

    const auto posFilter = _sympthomAndAnalis.indexOf(action);
    const auto posOrientation = _orientation.indexOf(action);

    if(posFilter != -1){
        _hide = posFilter;
        setOrientation(_render.orientation());
    }
    else if(posOrientation != -1){
        checking(_orientation);
        setOrientation((posOrientation == 0) ? Qt::Vertical : Qt::Horizontal);
    }
    else
        return;

    if(_hide != 0){
        drawHeader();
        drawRemedies();
    }
    else
        ui->spacefiller->setVisible(false);
}
void researchRemed::splitterMoved(int pos, int){
    int maxSplitterPos = 0, minSplitterPos = 0;
    ui->splitter->getRange(1, &minSplitterPos, &maxSplitterPos);

    if(pos == maxSplitterPos){
        _hide = 0;
        ui->spacefiller->setVisible(false);
    }
    else{
        const auto oldHide = _hide;
        _hide = (pos == minSplitterPos) ? 1 : 2;

        drawRemedies(true);

        if(oldHide == 0 && _hide == 2)
            drawHeader();

        if(ui->splitter->orientation() == Qt::Horizontal)
            ui->spacefiller->setVisible(true);
    }
}
inline qreal researchRemed::horizontalSize() const{
    auto font = QFont("default", 10);
    return QFontMetrics(font).height() * (1 + 0.5);
}
void researchRemed::hScrollBarMoved(int pos){
    const auto prevPos = ui->graphicsView_remedies->sceneRect();
    const auto horizon = horizontalSize();

    ui->graphicsView_header->setSceneRect(pos * horizon, 0, 1, 1);
    ui->graphicsView_remedies->setSceneRect(pos * horizon, prevPos.y(), 1, 1);
}
void researchRemed::vScrollBarMoved(int pos){
    const auto prevPos = ui->graphicsView_header->sceneRect();

    if(ui->splitter->orientation() == Qt::Horizontal){
        const auto data = _render.clipboardAndRemediesNumbers();
        const qint32 viewportHeigth = ui->listWidget->maximumViewportSize().height();
        qint32 height = 0;

        auto countPixels = [](auto & data, auto & _render, const auto pos){
            qint32 height = 0;

            for(auto i = data.second.cbegin(); i != data.second.cend(); ++i){
                if(std::distance(data.second.cbegin(), i) == pos + 1)
                    break;

                height += (*i) ? _render.clipboardHeight() : _render.symptomHeight();
            }

            return height;
        };

        height = countPixels(data, _render, pos);
        ui->listWidget->setCurrentRow(pos);

        if(viewportHeigth < height - _oldHeight){
            ++_beginPos;
            _oldHeight = countPixels(data, _render, _beginPos - 1);
            ui->graphicsView_remedies->setSceneRect(prevPos.x(), _oldHeight, 1, 1);
        }
        else if(height - _oldHeight <= 0){
            --_beginPos;
            _oldHeight = countPixels(data, _render, _beginPos - 1);
            ui->graphicsView_remedies->setSceneRect(prevPos.x(), _oldHeight, 1, 1);
        }
        else if(pos == 0){
            _beginPos = 0;
            _oldHeight = countPixels(data, _render, pos - 1);
            ui->graphicsView_remedies->setSceneRect(prevPos.x(), _oldHeight, 1, 1);
        }
    }
    else{
        if(!pos)
            ui->listWidget->setCurrentRow(0);

        const auto data = _render.clipboardAndRemediesNumbers();
        qsizetype clipCounter = 0, counter = -1;
        qint32 height = 0;

        for(auto i : data.second){
            if(i) ++clipCounter;
            else ++counter;

            if(counter == pos)
                break;

            if(counter != -1)
                height += (i) ? _render.clipboardHeight() : _render.symptomHeight();
        }

        ui->graphicsView_remedies->setSceneRect(prevPos.x(), height, 1, 1);
        ui->graphicsView_count->setSceneRect(0, height, 1, 1);
        ui->listWidget->setCurrentRow(clipCounter + pos);
    }
}
void researchRemed::closeClipboard(QLabel * closeButton){
    auto findIter = std::find(_labels.cbegin(), _labels.cend(), closeButton);
    auto showClip = _render.showedClipboards();

    showClip.at(findIter - _labels.cbegin()) = false;
    _render.setShowedClipboards(showClip);

    if(!isHidden())
        //setClipboards(showClip);
        show(showClip);
}
void researchRemed::resizeEvent(QResizeEvent * event){
    event->accept();

    if(_hide && ui->listWidget->count() != 0)
        drawRemedies(true);
}
void researchRemed::openListMenu(const QPoint & point){
    auto index = ui->listWidget->itemAt(point);//TODO: test functionality
    auto globalPos = ui->listWidget->mapToGlobal(point);

    if(index != nullptr && ui->listWidget->itemWidget(index) == nullptr)
        _listMenu->exec(globalPos);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void researchRemed::openProperty(){
    auto item = ui->listWidget->currentItem();

    auto setRmd = new setRemedy(_clipRemed,
        item->data(Qt::UserRole).value<std::pair<qsizetype, qsizetype>>(), this);

    setRmd->setAttribute(Qt::WA_DeleteOnClose);
    connect(setRmd, &setRemedy::clipboardRemedChanged, this, &researchRemed::setClipboardRemed);
    setRmd->exec();
}
void researchRemed::deleteSymptom(){
    auto item = ui->listWidget->currentItem();
    const auto symptomPos = item->data(Qt::UserRole).value<std::pair<qsizetype, qsizetype>>();
    _clipRemed->at(std::get<0>(symptomPos)).remove(std::get<1>(symptomPos));

    if(!isHidden())
        setClipboardRemed();
}
researchRemed::~researchRemed(){
    delete ui;
}
