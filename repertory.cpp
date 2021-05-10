#include "repertory.h"

QDir repertory::getRepDir() const noexcept{
    return _filename;
}
void repertory::changeFilter(QAction * action){
    if(action != nullptr){
        auto menu = qobject_cast<QMenu*>(action->parent());

        if(menu == nullptr)
            return;

        auto array = menu->actions();
        _remFilter = (quint16) action->data().toInt();
        menu->setTitle(action->text());

        for(auto & it : array){
            if(it != action){
                it->setChecked(false);
            }
        }

        if(!action->isChecked())
            action->setChecked(true);
        else
            repaintView();
    }
}
repertory::repertory(const QDir & filename, const QDir & system, std::shared_ptr<cache> & ch, const quint16 remFilter, QWidget *parent) : QWidget(parent)
{
    _filename = filename;
    _system = system;
    _codec = QTextCodec::codecForName("system");
    _symptom.open(filename.filePath("symptom").toStdString());
    _cache = ch;
    _remFilter = remFilter;

    auto vlayout = new QVBoxLayout(this);
    auto hlayout = new QHBoxLayout(this);
    _scene = new customScene(this);

    _viewLeft = new QGraphicsView(_scene, this);
    _viewRight = new QGraphicsView(_scene, this);

    _label = new QLabel(this);
    _label->close();
    _label->setFont(QFont("default", 10));

    openCtree view(filename.filePath("view").toStdString());
    auto barm = new QMenuBar(this);
    _menu = new QMenu("Full remed", barm);
    barm->addMenu(_menu);

    auto frm = _menu->addAction("Full remed");
    frm->setCheckable(true);
    frm->setData(-1);

    if(_remFilter == (quint16)-1){
        changeFilter(frm);
    }

    auto mask = 1;

    for(auto it = 0; it != view.size(); ++it){
        auto str = view.next();
        auto ps = _menu->addAction(_codec->toUnicode(str.substr(view.serviceDataLenght(), str.find('0', view.serviceDataLenght())).c_str()));
        ps->setData(mask);
        ps->setCheckable(true);

        if(mask == _remFilter){
            changeFilter(ps);
        }

        mask <<= 1;
    }
    connect(_menu, &QMenu::triggered, this, &repertory::changeFilter);

    _bar = new QScrollBar(Qt::Vertical, this);
    _bar->setMinimum(0);
    _bar->setMaximum(_symptom.size() - 1);

    hlayout->addWidget(_viewLeft);
    hlayout->addWidget(_viewRight);
    hlayout->addWidget(_bar);

    vlayout->addWidget(barm);
    vlayout->addWidget(_label);
    vlayout->addLayout(hlayout);
    setLayout(vlayout);

    _viewLeft->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    _viewRight->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    _viewLeft->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    _viewLeft->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    _viewRight->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    _viewRight->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    _viewLeft->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    _viewRight->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);

    _scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    _bar->setTracking(false);
    connect(_bar, &QScrollBar::valueChanged, this, &repertory::changedPos);
    connect(_scene, &customScene::labelActivated, this, &repertory::clickedAction);
    _symptom.back(false);
    _endIndex = QByteArray::fromStdString(_symptom.key());
    _symptom.front(false);
}
void repertory::repaintView(){
    renderingView(_viewLeft->height() * 2, _viewLeft->width());

    _viewLeft->setSceneRect(0, 0, 1, 1);
    _viewRight->setSceneRect(0, _viewLeft->height(), 1, 1);

    auto threadDraw = [](auto * draw){ draw->viewport()->update(); };
    QtConcurrent::run(threadDraw, _viewRight);
    QtConcurrent::run(threadDraw, _viewLeft);
}
void repertory::resizeEvent(QResizeEvent * event){
    event->ignore();
    repaintView();
}
void repertory::changedPos(const int pos){
    const auto str = QByteArray::fromStdString(_symptom.at(pos));
    _index = QByteArray::fromStdString(_symptom.key());
    const auto label = renderingLabel(str, _symptom);

    if(label.isEmpty()){
        _label->clear();
        _label->close();
    }
    else{
        _label->setText(label);
        _label->show();
    }

    repaintView();
}
void repertory::clickedAction(const QGraphicsSimpleTextItem * item){
    if(!item->data(0).isValid())
        return;

    QWidget * widget = nullptr;

    switch (item->data(0).toInt()) {
        case 0 :{
            widget = new Label(_cache, _filename,
                                     _system , item->data(1).toByteArray(), _remFilter, this);
            break;
        }
        case 1 :
            break;
        case 2 :{
            break;
        }
        case 3 : {
            widget = new remed_author(_filename, _system, _cache, item->data(2).toByteArray()
                                          , _remFilter, item->data(1).toUInt(), this);
            break;
        }
        case 4 : {
            widget = new author(_system, item->data(1).toUInt(), _cache, this);
            break;
        }
    }

    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->show();
}
void repertory::setPosition(const QByteArray & pos){
    _symptom.at(pos.toStdString(), false);
    const auto tp = _symptom.currentPosition();
    _bar->setValue(tp);
}
