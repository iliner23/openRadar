#include "researchremed.h"
#include "ui_researchremed.h"

researchRemed::researchRemed(QStringList clipNames, const std::shared_ptr<func::cache> & cache, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::researchRemed){
    ui->setupUi(this);
    _clipNames = clipNames;
    _scene = new QGraphicsScene(this);
    _render.reset(cache);
}
void researchRemed::renameLabels(){
    for(auto i = 0; i != 10; ++i)
        _labels[i].label->setText(_clipNames.at(i));
}
void researchRemed::setClipboards(std::array<bool, 10> act){
    ui->listWidget->clear();
    _render.setShowedClipboards(act);
    const auto clipboards = _render.clipboards();

    bool first = true;

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

            if(first){
                const auto ptr = ui->listWidget->count() - 1;

                _render.setClipboardHeight(widget->height());
                _render.setSymptomHeight(ui->listWidget->sizeHintForRow(ptr));
                first = false;
            }
        }
    }

    //INFO : only for test
    _render.setOrientation(Qt::Vertical);
    _render.setAnalysisType(researchRemedRender::showType::waffle);
    _render.setStrategyType(researchRemedRender::strategy::sumRemeds);

    _scene->addItem(_render.render(ui->graphicsView->size()));
}
void researchRemed::setClipboardName(QStringList name){
    _clipNames = name;

    if(!isHidden())
        setClipboards(_render.showedClipboards());
}
void researchRemed::setClipboardRemed(std::array<QVector<rci>, 10> array){
    _render.setClipboards(array);

    if(!isHidden())
        setClipboards(_render.showedClipboards());
}
researchRemed::~researchRemed(){
    delete ui;
}
