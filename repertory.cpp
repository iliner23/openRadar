#include "repertory.h"

void repertory::changeFilter(QAction * action){
    if(action != nullptr){
        auto menu = qobject_cast<QMenu*>(action->parent());

        if(menu == nullptr)
            return;

        auto array = menu->actions();
        _engine.setChaptersFilter((quint16) action->data().toInt());
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
repertory::repertory(const QDir filename, const QDir system,
    std::shared_ptr<cache> & ch, const std::pair<QLocale::Language, QLocale::Language> lang, const quint16 remFilter, QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    _lang = lang;//TODO : for working vocabulary

    _codec = QTextCodec::codecForName(languages::chooseCodec(_lang));
    _filename = filename;
    _system = system;
    _cache = ch;

    auto vlayout = new QVBoxLayout(this);
    auto hlayout = new QHBoxLayout(this);
    _scene = new customScene(this);

    _engine.reset(filename, _cache, _codec);
    _engine.setChaptersFilter(remFilter);
    _engine.setCurrentPosition(0);

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

    if(_engine.chaptersFilter() == (quint16)-1){
        changeFilter(frm);
    }

    auto mask = 1;

    for(auto it = 0; it != view.size(); ++it){
        auto str = view.next();
        auto ps = _menu->addAction(_codec->toUnicode(str.substr(view.serviceDataLenght(), str.find('0', view.serviceDataLenght())).c_str()));
        ps->setData(mask);
        ps->setCheckable(true);

        if(mask == _engine.chaptersFilter()){
            changeFilter(ps);
        }

        mask <<= 1;
    }
    _bar = new QScrollBar(Qt::Vertical, this);
    _bar->setMinimum(0);
    _bar->setMaximum(_engine.chapterSize() - 1);

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

    _viewLeft->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    _viewRight->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

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
    _scene->clear();
    auto group = _engine.render(QSize(_viewLeft->width(), _viewLeft->height() * 2));

    for(auto & it : group)
        _scene->addItem(it);

    _viewLeft->setSceneRect(0, 0, _viewLeft->width(), _viewLeft->height());
    _viewRight->setSceneRect(0, _viewLeft->height(), _viewRight->width(), _viewRight->height());
}
void repertory::repaintView(){
    rendering();
    const auto labelsVec = _engine.navigationItems();

    if(_pointer.isEmpty())
        _pointer = _engine.currentKey();

    auto pred = [&](const auto & it){
        return (it->data(1).toByteArray() == _pointer) ? true : false;
    };

    auto iter = std::find_if(labelsVec.cbegin(), labelsVec.cend(), pred);

    if(iter != labelsVec.cend()){
        for(auto & it : labelsVec){
            if(it->data(1).toByteArray() != _pointer)
                it->hide();
            else
                it->show();
        }
    }
}
void repertory::resizeEvent(QResizeEvent * event){
    event->ignore();
    repaintView();
}
void repertory::changedPos(const int pos){
    const auto oldKey = _engine.currentKey();
    _engine.setCurrentPosition(pos);
    const auto newIndex = _engine.currentKey();

    const auto label = _engine.renderingLabel(true);
    auto oldVis = _label->isVisible();

    _label->setText(label);
    _label->setVisible(!label.isEmpty());

    auto pred = [&](const auto & it){
        return (it->data(1).toByteArray() == newIndex) ? true : false;
    };

    auto labelsVec = _engine.navigationItems();

    if(oldVis != _label->isVisible()){
        _viewLeft->setSceneRect(0, 0, _viewLeft->width(), _viewLeft->height());

        if(_label->isVisible())
            _viewRight->setSceneRect(0, _viewLeft->height(), _viewRight->width(), _viewRight->height());
        else
            _viewRight->setSceneRect(0, _viewLeft->height() + _label->height(), _viewRight->width(), _viewRight->height());
    }

    if(std::find_if(labelsVec.cbegin(), labelsVec.cend(), pred) != labelsVec.cend()){
        _engine.setCurrentKey(oldKey);
        _pointer = newIndex;

        for(auto & it : labelsVec){
            if(it->data(1).toByteArray() != _pointer)
                it->hide();
            else
                it->show();
        }
    }
    else{
        _engine.setCurrentKey(newIndex);
        _pointer.clear();
        repaintView();
    }
}
void repertory::clickedAction(QGraphicsSimpleTextItem * item){
    if(!item->data(0).isValid())
        return;

    if(item->data(0) != 5 && item->data(0) != 0)
        return;

    _pointer = item->data(1).toByteArray();
    const auto oldKey = _engine.currentKey();
    _engine.setCurrentKey(_pointer);
    const auto posit = _engine.currentPosition();
    _engine.setCurrentKey(oldKey);
    _bar->setValue(posit);
}
void repertory::doubleClickedAction(QGraphicsSimpleTextItem * item){
    if(!item->data(0).isValid())
        return;

    QWidget * widget = nullptr;

     //0 - label num, 1 - (see, 2 - links, 3 - remed, 4 - author
    switch (item->data(0).toInt()) {
        case 0 :{
            auto label = new Label(_cache, _filename,
                item->data(1).toByteArray(), _engine.chaptersFilter(), _lang, this);

            if(label->isHiddenLabels()){
                notShowLabel();
                label->deleteLater();
                return;
            }

            widget = label;
            break;
        }
        case 1 :
        case 2 :{//TODO : add localize support for searching
            auto mesBox = [&](){
                QMessageBox mbox(this);
                mbox.setText("Позиция не найдена");
                mbox.exec();
            };

            try{
                QStringList epList;
                quint8 digit = 0;
                QRegularExpression reg(R"(\s*(\d*)([\w\s][^-\n\r]+))");
                auto glMatch = reg.globalMatch(item->data(1).toString());

                while(glMatch.hasNext()){
                    auto match = glMatch.next();
                    const auto dg = match.captured(1);

                    if(!dg.isEmpty())
                        digit = dg.toUShort();

                    auto str = match.captured(2);

                    while(str.back() == ' ')
                        str.chop(1);

                    epList += str;
                }

                auto symFile = _engine.symptomFile();
                symFile.at(item->data(2).toByteArray().toStdString());

                QByteArrayList originalPos;

                if(digit > 1){
                    originalPos = functions::getRootPath(symFile);
#if QT_VERSION >= 0x060000
                    originalPos.remove(0, originalPos.size() - digit + 1);
#else
                    const auto size = originalPos.size() - digit + 1;

                    if(size > 0){
                        for(auto i = 0; i != size; ++i)
                            originalPos.removeFirst();
                    }
                    else
                        originalPos.clear();
#endif
                }

                auto iterEpList = epList.begin();

                symFile.setIndex(1);

                while(true){
                    const auto size = iterEpList->size() - 20;

                    if(size > 0)
                        iterEpList->chop(size);

                    const auto encodeKey = ((!originalPos.isEmpty()) ? originalPos.front().toStdString() : std::string(6, '\0')) +
                                            symFile.encodeKey(iterEpList->toUpper().toStdString());

                    symFile.firstFind(encodeKey, false);

                    while(true){
                        auto iterFile = symFile;
                        auto keyPos = symFile.currentValue().substr(0, 6);
                        std::reverse(keyPos.begin(), keyPos.end());

                        iterFile.setIndex(0);
                        iterFile.at(keyPos, false);

                        const auto pos = functions::getRootPath(iterFile);

                        if(pos.size() != originalPos.size() + 1){
                            symFile.next(false);
                            continue;
                        }

                        if(!std::equal(pos.crbegin(), pos.crend() - 1, originalPos.crbegin())){
                            symFile.next(false);
                            continue;
                        }

                        const auto val = functions::renderingLabel(iterFile, _codec);

                        if(val.first.front().indexOf(*iterEpList, 0, Qt::CaseInsensitive) != -1){
                            if(pos.size() == digit - 1 + epList.size()){
                                setPosition(pos.first());
                                return;
                            }

                            originalPos.prepend(pos.first());
                            ++iterEpList;

                            break;
                        }
                        else{
                            mesBox();
                            return;
                        }
                    }
                }
            } catch(...){
              mesBox();
              return;
            }
        }
        case 3 : {
            widget = new remed_author(_filename, _cache, item->data(2).toByteArray()
                                          , _engine.chaptersFilter(), item->data(1).toUInt(), _codec, this);
            break;
        }
        case 4 : {
            widget = new author(item->data(1).toUInt(), _cache, _codec, this);
            break;
        }
        default:
            return;
    }

    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->show();
}
void repertory::setPosition(const QByteArray pos){
    _engine.setCurrentKey(pos);
    _bar->setValue(_engine.currentPosition());
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

        auto lab = new Label(_cache, _filename,
                    _pointer, _engine.chaptersFilter(), _lang, this);

        if(lab->isHiddenLabels()){
            notShowLabel();
            lab->deleteLater();
            return;
        }

        lab->setAttribute(Qt::WA_DeleteOnClose);
        lab->show();
        return;
    }

    QWidget::keyPressEvent(event);
}
void repertory::notShowLabel() const{
    QMessageBox msg;
    msg.setText("Этот симптом не существует в текущем уровне доверия репертория");
    msg.exec();
}
void repertory::openVocabulary(){
    if(_vocabulary == nullptr){
        _vocabulary = new vocabulary(_system, _lang, _filename, _codec, this);
        connect(_vocabulary, &vocabulary::sendKey, this, &repertory::setPosition);
    }

    _vocabulary->showNormal();
    QApplication::setActiveWindow(_vocabulary);
}
