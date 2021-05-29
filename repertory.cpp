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
        _engine->setChaptersFilter((quint16) action->data().toInt());
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
    std::shared_ptr<cache> & ch, QTextCodec * codec, const quint16 remFilter, QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    _codec = codec;
    _filename = filename;
    _system = system;
    _cache = ch;

    auto vlayout = new QVBoxLayout(this);
    auto hlayout = new QHBoxLayout(this);
    _scene = new customScene(this);

    _engine = new repertoryEngine(filename, _cache, _scene, _codec);
    _engine->setChaptersFilter(remFilter);
    _engine->setCurrentPosition(0);
    _engine->setUsingNavigationItems(true);

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

    if(_engine->chaptersFilter() == (quint16)-1){
        changeFilter(frm);
    }

    auto mask = 1;

    for(auto it = 0; it != view.size(); ++it){
        auto str = view.next();
        auto ps = _menu->addAction(_codec->toUnicode(str.substr(view.serviceDataLenght(), str.find('0', view.serviceDataLenght())).c_str()));
        ps->setData(mask);
        ps->setCheckable(true);

        if(mask == _engine->chaptersFilter()){
            changeFilter(ps);
        }

        mask <<= 1;
    }
    _bar = new QScrollBar(Qt::Vertical, this);
    _bar->setMinimum(0);
    _bar->setMaximum(_engine->chaptersSize() - 1);

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

    connect(_menu, &QMenu::triggered, this, &repertory::changeFilter);
    connect(_bar, &QScrollBar::valueChanged, this, &repertory::changedPos);
    connect(_scene, &customScene::doubleClick, this, &repertory::doubleClickedAction);
    connect(_scene, &customScene::click, this, &repertory::clickedAction);

    _viewLeft->setFocusPolicy(Qt::NoFocus);
    _viewRight->setFocusPolicy(Qt::NoFocus);
}
void repertory::rendering(){
    _engine->render(_viewLeft->height() * 2, _viewLeft->width());
    _viewLeft->setSceneRect(0, 0, _viewLeft->width(), _viewLeft->height());
    _viewRight->setSceneRect(0, _viewRight->height(), _viewRight->width(), _viewRight->height());
}
void repertory::redrawing(){
    auto threadDraw = [](auto * draw){ draw->viewport()->update(); };
    auto fut1 = QtConcurrent::run(threadDraw, _viewRight);
    auto fut2 = QtConcurrent::run(threadDraw, _viewLeft);
    fut1.waitForFinished();
    fut2.waitForFinished();
}
void repertory::repaintView(){
    rendering();
    const auto labelsVec = _engine->navigationItems();

    if(_pointer.isEmpty())
        _pointer = _engine->currentKey();

    auto pred = [&](const auto & it){
        return (it->data(1).toByteArray() == _pointer) ? true : false;
    };

    auto iter = std::find_if(labelsVec.cbegin(), labelsVec.cend(), pred);

    if(iter != labelsVec.cend()){
        for(auto & it : labelsVec){
            if(it->data(1).toByteArray() != (*iter)->data(1).toByteArray())
                it->hide();
            else
                it->show();
        }
    }

    redrawing();
}
void repertory::resizeEvent(QResizeEvent * event){
    event->ignore();
    repaintView();
}
void repertory::changedPos(const int pos){
    const auto oldKey = _engine->currentKey();
    _engine->setCurrentPosition(pos);
    const auto newIndex = _engine->currentKey();

    const auto label = _engine->renderingLabel(true);
    const bool oldVis = _label->isVisible();
    auto labelsVec = _engine->navigationItems();

    _engine->setCurrentKey(oldKey);

    _label->setText(label);
    _label->setVisible(!label.isEmpty());

    auto pred = [&](const auto & it){
        return (it->data(1).toByteArray() == newIndex) ? true : false;
    };

    if(oldVis != _label->isVisible()){
        auto iter = std::find_if(labelsVec.cbegin(), labelsVec.cend(), pred);
        _pointer = newIndex;

        if(iter == labelsVec.cend() ||
                _scene->height() - _label->height() <= (*iter)->y() + (*iter)->boundingRect().height()){
             _engine->setCurrentKey(newIndex);
        }

        rendering();
        labelsVec = _engine->navigationItems();

        for(auto & it : labelsVec){
            if(it->data(1).toByteArray() != _pointer)
                it->hide();
            else
                it->show();
        }

        redrawing();
    }
    else{
        auto iter = std::find_if(labelsVec.cbegin(), labelsVec.cend(), pred);

        if(iter != labelsVec.cend()){
            _pointer = newIndex;
            redrawPointer(*iter, labelsVec);
        }
        else{
            _engine->setCurrentKey(newIndex);
            _pointer.clear();
            repaintView();
        }
    }
}
void repertory::redrawPointer(QGraphicsItem * item, const QVector<QGraphicsItem*> & labelsVec){
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

    for(auto & it : labelsVec){
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
    const auto oldKey = _engine->currentKey();
    _engine->setCurrentKey(_pointer);
    const auto posit = _engine->currentPosition();
    _engine->setCurrentKey(oldKey);
    _bar->setValue(posit);
}
void repertory::doubleClickedAction(QGraphicsSimpleTextItem * item){
    if(!item->data(0).isValid())
        return;

    QWidget * widget = nullptr;

    switch (item->data(0).toInt()) {
        case 0 :{
            widget = new Label(_cache, _filename,
                                     _system , item->data(1).toByteArray(), _engine->chaptersFilter(), _codec, this);
            break;
        }
        case 1 :
            return;
        case 2 :{
            return;
        }
        case 3 : {
            widget = new remed_author(_filename, _system, _cache, item->data(2).toByteArray()
                                          , _engine->chaptersFilter(), item->data(1).toUInt(), this);
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
    _engine->setCurrentKey(pos);
    _bar->setValue(_engine->currentPosition());
}
QTextCodec * repertory::getTextCodec() const noexcept{
    return _codec;
}
void repertory::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_Up){
        _bar->setValue(_bar->value() - 1);
        return;
    }
    if(event->key() == Qt::Key_Down){
        _bar->setValue(_bar->value() + 1);
        return;
    }
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return){
        if(_pointer.isNull())
            return;

        auto lab = new Label(_cache, _filename, _system ,
                    _pointer, _engine->chaptersFilter(), _codec, this);
        lab->setAttribute(Qt::WA_DeleteOnClose);
        lab->show();
        return;
    }

    QWidget::keyPressEvent(event);
}
