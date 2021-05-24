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
        _render.remFilter = (quint16) action->data().toInt();
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
repertory::repertory(const QDir & filename, const QDir & system,
    std::shared_ptr<cache> & ch, QTextCodec * codec, const quint16 remFilter, QWidget *parent) : QWidget(parent), abstractEngine(codec)
{
    _filename = filename;
    _system = system;
    _symptom.open(filename.filePath("symptom").toStdString());
    _cache = ch;
    _render.remFilter = remFilter;

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

    if(_render.remFilter == (quint16)-1){
        changeFilter(frm);
    }

    auto mask = 1;

    for(auto it = 0; it != view.size(); ++it){
        auto str = view.next();
        auto ps = _menu->addAction(_codec->toUnicode(str.substr(view.serviceDataLenght(), str.find('0', view.serviceDataLenght())).c_str()));
        ps->setData(mask);
        ps->setCheckable(true);

        if(mask == _render.remFilter){
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
    connect(_scene, &customScene::doubleClick, this, &repertory::doubleClickedAction);
    connect(_scene, &customScene::click, this, &repertory::clickedAction);

    _symptom.back(false);
    _render.endIndex = QByteArray::fromStdString(_symptom.key());
    _symptom.front(false);
}
void repertory::repaintView(){
    renderingView(_viewLeft->height() * 2, _viewLeft->width());

    if(_pointer.isEmpty())
        _pointer = _render.index;

    auto pred = [&](const auto & it){
        return (it->data(1).toByteArray() == _pointer) ? true : false;
    };

    auto iter = std::find_if(_render.labelsVec.cbegin(), _render.labelsVec.cend(), pred);

    if(iter != _render.labelsVec.cend())
        redrawPointer(*iter);

    _viewLeft->setSceneRect(0, 0, _viewLeft->width(), _viewLeft->height());
    _viewRight->setSceneRect(0, _viewRight->height(), _viewRight->width(), _viewRight->height());

    auto threadDraw = [](auto * draw){ draw->viewport()->update(); };
    auto fut1 = QtConcurrent::run(threadDraw, _viewRight);
    auto fut2 = QtConcurrent::run(threadDraw, _viewLeft);
    fut1.waitForFinished();
    fut2.waitForFinished();
}
void repertory::resizeEvent(QResizeEvent * event){
    event->ignore();
    repaintView();
}
void repertory::changedPos(const int pos){
    const auto oldIndex = _render.index;

    _symptom.at(pos, false);
    _render.index = QByteArray::fromStdString(_symptom.key());

    const auto label = renderingLabel(true);
    const bool oldVis = _label->isVisible();

    _label->setText(label);
    _label->setVisible(!label.isEmpty());

    auto pred = [&](const auto & it){
        return (it->data(1).toByteArray() == _render.index) ? true : false;
    };

    auto iter = std::find_if(_render.labelsVec.cbegin(), _render.labelsVec.cend(), pred);

    if(iter != _render.labelsVec.cend()){
        if(oldVis == _label->isVisible()){
            redrawPointer(*iter);
            _pointer = _render.index;
            _render.index = oldIndex;
        }
        else{
            _pointer = _render.index;
            _render.index = oldIndex;
            repaintView();
        }
    }
    else{
        _pointer.clear();
        repaintView();
    }
}
void repertory::redrawPointer(QGraphicsSimpleTextItem * item){
    auto redraw = [&](const auto & it, const auto height){
        if(it->y() < height){
            _viewLeft->viewport()->update
                    (it->x(), it->y(),
                     it->boundingRect().width(), it->boundingRect().height());
        }
        else{
            _viewRight->viewport()->update
                    (it->x(), it->y() - height,
                     it ->boundingRect().width(), it->boundingRect().height());
        }
    };

    const auto key = item->data(1).toByteArray();

    for(auto & it : _render.labelsVec){
        if(it->data(1).toByteArray() != key){
            it->hide();
            redraw(it, _viewLeft->height());
        }
        else{
            it->show();
            redraw(it, _viewLeft->height());
        }
    }
}
void repertory::clickedAction(QGraphicsSimpleTextItem * item){
    if(!item->data(0).isValid())
        return;

    if(item->data(0) != 5 && item->data(0) != 0)
        return;

    _pointer = item->data(1).toByteArray();
    _symptom.at(_pointer.toStdString(), false);
    _bar->setValue(_symptom.currentPosition());
}
void repertory::doubleClickedAction(QGraphicsSimpleTextItem * item){
    if(!item->data(0).isValid())
        return;

    QWidget * widget = nullptr;

    switch (item->data(0).toInt()) {
        case 0 :{
            widget = new Label(_cache, _filename,
                                     _system , item->data(1).toByteArray(), _render.remFilter, _codec, this);
            break;
        }
        case 1 :
            return;
        case 2 :{
            return;
        }
        case 3 : {
            widget = new remed_author(_filename, _system, _cache, item->data(2).toByteArray()
                                          , _render.remFilter, item->data(1).toUInt(), this);
            break;
        }
        case 4 : {
            widget = new author(_system, item->data(1).toUInt(), _cache, this);
            break;
        }
        default:
            return;
    }

    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->show();
}
void repertory::setPosition(const QByteArray & pos){
    _symptom.at(pos.toStdString(), false);
    const auto tp = _symptom.currentPosition();
    _bar->setValue(tp);
}
QTextCodec * repertory::getTextCodec() const noexcept{
    return _codec;
}
