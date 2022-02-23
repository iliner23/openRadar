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
}
void researchRemed::renameLabels(){
    for(auto i = 0; i != 10; ++i)
        _labels[i].label->setText(_clipNames.at(i));
}
void researchRemed::drawScene(){
    _scene->clear();
    auto var = _render.render(ui->graphicsView->size());
    _scene->setSceneRect(0, 0, 0, 0);
    _scene->addItem(var);
    ui->widget->setMinimumHeight(_render.labelHeight());
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
            ui->listWidget->addItem(func::renderingLabel(data, false, it.codec));
        }

        _render.setClipboardHeight(ui->listWidget->sizeHintForRow(0));
        _render.setSymptomHeight(ui->listWidget->sizeHintForRow(1));
    }

    //NOTE : only for test
    _render.setAnalysisType(researchRemedRender::showType::waffle);
    _render.setStrategyType(researchRemedRender::strategy::sumRemeds);

    setOrientation(_render.orientation());
    //qDebug() << ui->graphicsView->size();//TODO : fix wrong size
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
    auto layout = qobject_cast<QGridLayout*>(ui->horizontalLayoutWidget->layout());
    _render.setOrientation(orien);

    if(orien == Qt::Horizontal){
        layout->removeItem(ui->verticalLayout_4);
        layout->removeWidget(ui->graphicsView);
        ui->widget->hide();

        layout->addItem(ui->verticalLayout_4, 1, 0);
        layout->addWidget(ui->graphicsView, 2, 0);
        layout->setColumnStretch(0,0);
        layout->setColumnStretch(2,0);
        layout->setRowStretch(1,1);
        layout->setRowStretch(2,1);
    }
    else{
        layout->removeItem(ui->verticalLayout_4);
        layout->removeWidget(ui->graphicsView);

        ui->widget->show();

        layout->addItem(ui->verticalLayout_4, 1, 0);
        layout->addWidget(ui->graphicsView, 1, 2);
        layout->setRowStretch(1,0);
        layout->setRowStretch(2,0);
        layout->setColumnStretch(0,3);
        layout->setColumnStretch(2,8);
    }
}
void researchRemed::testOrien(){
    setOrientation((_render.orientation() == Qt::Horizontal) ? Qt::Vertical : Qt::Horizontal);//NOTE: it's test. Need delete after
    drawScene();
}
researchRemed::~researchRemed(){
    delete ui;
}
