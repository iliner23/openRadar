#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _chapters = new windowChapters(this);

    {
        QStringList clip = {
            "Клипборд 1",
            "Клипборд 2",
            "Клипборд 3",
            "Клипборд 4",
            "Клипборд 5",
            "Клипборд 6",
            "Клипборд 7",
            "Клипборд 8",
            "Клипборд 9",
            "Клипборд 10",
        };

        _clipNames = std::make_shared<QStringList>(std::move(clip));
        _clipboadrs = std::make_shared<std::array<QVector<rci>, 10>>();
    }

    _catalog.open(QDir(func::dataPath % "../system").filePath("catalog").toStdString());
    auto dataDirs = QDir(func::dataPath % "../data").entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    auto keysDirs = QDir(func::dataPath % "../data/keynotes").entryInfoList(QDir::Files);
    const QStringList tpCmp = {"view", "word1", "word2", "chapter", "extract", "symptom"};

    QTextCodec * cd = QTextCodec::codecForName(lang::defaultCodec());

    QStringList reperts;
    QStringList keys;
    QVector<QDir> keysPos;

    for(auto it = 0; it != _catalog.size(); ++it){
        auto str = QByteArray::fromStdString(_catalog.next());

        if(str.at(4) == 1){
            for(auto & ir : dataDirs){
                QString st = func::dataPath % str.mid(str.lastIndexOf('\0', str.size() - 2) + 1);
                //NOTE: work directory for repertory
                st.chop(2);
                QFileInfo dir(st);

                if(dir == ir){
                    const auto reclen = _catalog.serviceDataLenght();
                    const auto tp = str.indexOf('\0', reclen);
                    bool exit = false;

                    for(auto & iqcmp : tpCmp){
                        auto file = dir;
                        file.setFile(file.filePath() % "/" % iqcmp % ".dat");

                        if(!file.exists()){
                            exit = true;
                            break;
                        }
                    }

                    if(exit)
                        continue;

                    const auto nextTp = str.indexOf('\0', tp + 1);

                    auto nextLangPos = nextTp;
                    nextLangPos = str.indexOf('\0', nextLangPos + 1);
                    nextLangPos = str.indexOf('\0', nextLangPos + 1);

                    const auto orig = str.at(nextLangPos + 1);
                    const auto local = str.at(nextLangPos + 2);

                    auto lang = std::make_pair(QLocale(), QLocale());

                    if(orig != 0 && orig > 0 && orig <= (char) lang::radarLang.size())
                        lang.first = lang::radarLang.at(orig - 1);
                    else
                        continue;

                    if(local != 0 && local > 0 && local <= (char) lang::radarLang.size())
                        lang.second = lang::radarLang.at(local - 1);
                    else
                        lang.second = lang.first;

                    if(lang.first == QLocale::AnyLanguage)
                        continue;

                    const auto nativeCodec = lang::langToName(lang.first);
                    const auto localizeCodec = lang::langToName(lang.second);

                    if(lang.second != QLocale::AnyLanguage &&
                            nativeCodec != lang::defaultCodec() &&
                             localizeCodec != lang::defaultCodec() &&
                            nativeCodec != localizeCodec)
                        continue;

                    _repertsLang.push_back(lang);

                    auto text = cd->toUnicode(str.mid(tp + 1, nextTp - tp - 1));
                    text += " (" % cd->toUnicode(str.mid(reclen, tp - reclen)) % ") ";

                    reperts += text;
                    _repertsPos += dir.filePath();
                }
            }
        }
        else if(str.at(4) == 2){
            for(auto & ir : keysDirs){
                QString st = func::dataPath % str.mid(str.lastIndexOf('\0', str.size() - 2) + 1);
                //NOTE: work directory for keys
                st.chop(1);
                QFileInfo dir(st + ".dat");

                if(dir == ir){
                    const auto reclen = _catalog.serviceDataLenght();
                    const auto tp = str.indexOf('\0', reclen);

                    auto name = cd->toUnicode(
                        str.mid(tp + 1, str.indexOf('\0', tp + 1) - tp - 1));
                    name += " (" % cd->toUnicode(str.mid(reclen, tp - reclen)) % ") ";
                    keys += name;

                    keysPos.append(dir.path() + QDir::separator() + dir.completeBaseName());
                }
            }
        }
        else
            break;
    }

    openCtree remed(QDir(func::dataPath % "../system").filePath("remed").toStdString());
    openCtree author(QDir(func::dataPath % "../system").filePath("author").toStdString());

    _cache = std::make_shared<func::cache>();
    _cache->_lenRem = remed.serviceDataLenght();
    _cache->_lenAuthor = author.serviceDataLenght();

    _cache->_cacheRemed.reserve(remed.size());
    _cache->_cacheAuthor.reserve(author.size());

    for(auto it = 0; it != remed.size(); ++it){
        auto tp = remed.next();
        auto key = remed.key();
        quint16 kt;
        ((char *)&kt)[0] = key[1];
        ((char *)&kt)[1] = key[0];
        _cache->_cacheRemed[kt] = std::move(tp);
    }

    for(auto it = 0; it != author.size(); ++it){
        auto tp = author.next();
        auto key = author.key();
        quint16 kt;
        ((char *)&kt)[0] = key[1];
        ((char *)&kt)[1] = key[0];
        _cache->_cacheAuthor[kt] = std::move(tp);
    }

    _choose = new RepChose(reperts , _repertsPos, _repertsLang, this);
    _remedList = new keysRemedList(keysPos, keys, this);
    _keych = new KeyChoose(keys, _remedList, this);
    _takeRemed = new takeRemed(_clipNames, this);
    _research = new researchRemed(_clipNames , _clipboadrs, _cache, this);

    connect(_choose, &RepChose::chooseRep, this, &MainWindow::openRepertory);
    connect(_chapters, &windowChapters::activatedBranch, this, &MainWindow::setPositionInRepertory);

    //connect(_takeRemed, &takeRemed::changeClipboardsName, this, &MainWindow::setClipboardsName);
    connect(_takeRemed, &takeRemed::addedClipboardsRemed, this, &MainWindow::addClipboardsRemed);
    //connect(this, &MainWindow::changeClipboardsName, _takeRemed, &takeRemed::setClipboardsName);

    connect(this, &MainWindow::changeClipboardsRemed, _research, &researchRemed::setClipboardRemed);
    connect(this, &MainWindow::changeClipboardsName, _research, &researchRemed::setClipboardName);
    connect(_research, &researchRemed::clipboadrsDataChanged, this, &MainWindow::clipboadrsDataChanged);

    QAction * researchPtrs[] = {ui->clipboard1, ui->clipboard2, ui->clipboard3, ui->clipboard4,
                               ui->clipboard5, ui->clipboard6, ui->clipboard7, ui->clipboard8,
                               ui->clipboard9, ui->clipboard10};

    for(qsizetype it = 0; it != std::size(_clipboardsActions); ++it){
        _clipboardsActions[it] = new widgetAction();
        _clipboardsActions[it]->setDefaultAction(researchPtrs[it]);
        connect(_clipboardsActions[it], &widgetAction::clicked, this, [=](){ openResearchTest(it); });
        connect(_clipboardsActions[it], &widgetAction::sendDropKey, this, [=](auto key) { dragAndDropAddRemed(key, it); } );

        ui->toolBar_2->addWidget(_clipboardsActions[it]);
    }

    ui->toolBar_2->addSeparator();
    ui->toolBar_2->addAction(ui->openKeys);
}
void MainWindow::openTakeRemed(){
    if(ui->mdiArea->currentSubWindow() != nullptr){
        auto currentRep = qobject_cast<repertory*>(
                    ui->mdiArea->currentSubWindow()->widget());

        _takeRemed->setPos(currentRep->catalog(),
                           currentRep->currentPosition(), currentRep->getFilter(), currentRep->textCodec());
        _takeRemed->show();
    }
}
inline void MainWindow::changeIcon(qint8 pos, bool opened){
    QAction * researchPtrs[] = {ui->clipboard1, ui->clipboard2, ui->clipboard3, ui->clipboard4,
                               ui->clipboard5, ui->clipboard6, ui->clipboard7, ui->clipboard8,
                               ui->clipboard9, ui->clipboard10};

    const QString open[] = {":/icons/open/full/res/openFullRepertory",
                            ":/icons/open/empty/res/openEmptyRepertory"};
    const QString notOpen[] = {":/icons/notOpen/full/res/notOpenFullRepertory",
                               ":/icons/notOpen/empty/res/notOpenEmptyRepertory"};

    researchPtrs[pos]->setData(opened);

    if(!_clipboadrs->at(pos).isEmpty()){
        researchPtrs[pos]->setIcon(QIcon(
            ((opened) ? open[0] : notOpen[0]) % QString::number(pos + 1) % ".png"));
    }
    else
        researchPtrs[pos]->setIcon(QIcon(
            ((opened) ? open[1] : notOpen[1]) % QString::number(pos + 1) % ".png"));
}
void MainWindow::openResearchTest(qint8 pos){
    std::array<bool, 10> val;

    for(auto & it : val)
        it = false;

    if(_pressed){
        if(_pressedClipboard.isEmpty()){
            for(auto it = 0; it != 10; ++it)
                changeIcon(it, false);
        }

        if(pos != -1 && _pressedClipboard.indexOf(pos) == -1){
            _pressedClipboard.push_back(pos);
            changeIcon(pos, true);
        }
    }
    else if(!_pressedClipboard.isEmpty()){
        for(const auto & it : _pressedClipboard)
            val.at(it) = true;

        _research->show(val);
        _pressedClipboard.clear();
    }
    else if(pos != -1){
        val.at(pos) = true;

        for(auto it = 0; it != 10; ++it)
            changeIcon(it, false);

        changeIcon(pos, true);
        _research->show(val);
    }
}
void MainWindow::keyPressEvent(QKeyEvent * event){
    if(event->key() == Qt::Key_Control)
        _pressed = true;
}
void MainWindow::keyReleaseEvent(QKeyEvent * event){
    if(event->key() == Qt::Key_Control){
        _pressed = false;
        //openResearchTest(nullptr);
        openResearchTest(-1);
    }
}
MainWindow::~MainWindow(){
    delete ui;
}
void MainWindow::openRepertory(QModelIndex & item, const quint16 repLevel, std::pair<QLocale, QLocale> lang){
    auto index = item.row();
    auto rep = new repertory(QDir(_repertsPos.at(index)), QDir(func::dataPath % "system"),
                             _cache, lang, _remedList, repLevel, ui->mdiArea);
    rep->setAttribute(Qt::WA_DeleteOnClose);
    rep->setWindowTitle(item.data().toString());
    auto repPtr = ui->mdiArea->addSubWindow(rep, Qt::SubWindow);
    repPtr->setMinimumSize(500, 500);
    repPtr->show();
}
void MainWindow::openRepertoryWindow(){
    _choose->show();
}
void MainWindow::openKeysChooseWindow(){
    _remedList->close();
    _keych->exec();
}
void MainWindow::windowChanged(){
    auto act = ui->menu_6->actions();
    const auto size = act.size();

    for(auto i = 6; i < size; ++i){
        ui->menu_6->removeAction(act.at(i));
    }

    const auto sub = ui->mdiArea->activeSubWindow();
    const auto list = ui->mdiArea->subWindowList();

    ui->findSymptom->setEnabled(!list.isEmpty());
    ui->findFromCurrentSymptom->setEnabled(sub != nullptr);
    ui->externalSearch->setEnabled(sub != nullptr);
    ui->takeSymptom->setEnabled(sub != nullptr);
    ui->takeSymptom1->setEnabled(sub != nullptr);
    ui->takeSymptom2->setEnabled(sub != nullptr);
    ui->takeSymptom3->setEnabled(sub != nullptr);
    ui->takeSymptom4->setEnabled(sub != nullptr);

    for(auto & it : list){
        auto atr = ui->menu_6->addAction(it->windowTitle());
        connect(ui->menu_6, &QMenu::triggered, this, &MainWindow::windowActivated);

        atr->setCheckable(true);
        atr->setData(QVariant::fromValue(it));

        if(sub == it)
            atr->setChecked(true);
    }
}
void MainWindow::windowActivated(QAction * action){
    ui->mdiArea->setActiveSubWindow(action->data().value<QMdiSubWindow*>());
}
void MainWindow::openChapters(){
    _chapters->show(ui->mdiArea->subWindowList(), ui->mdiArea->activeSubWindow());
}
void MainWindow::openChaptersInCurrentPos(){
    const auto key =
            static_cast<class repertory*>
                (ui->mdiArea->currentSubWindow()->widget())->currentPosition();

    _chapters->show(ui->mdiArea->subWindowList(),
                    ui->mdiArea->activeSubWindow(),
                    key);
}
void MainWindow::setPositionInRepertory(const QModelIndex & pos, const qint32 winIndex){
    auto list = ui->mdiArea->subWindowList();
    if(!list.isEmpty() && winIndex < list.size()){
        auto window = qobject_cast<repertory*>(list.at(winIndex)->widget());
        window->setPosition(static_cast<const searchModel::_node *>(pos.internalPointer())->key().right(6));
    }
}
void MainWindow::openVocabulary(){
    auto rep = qobject_cast<repertory*>(ui->mdiArea->activeSubWindow()->widget());
    rep->openVocabulary();
}
/*void MainWindow::setClipboardsName(){
    emit changeClipboardsName();
}*/
void MainWindow::addClipboardsRemed(func::remedClipboardInfo remed, quint8 clip){
    auto & clp = _clipboadrs->at(clip);

    auto pred = [&](const auto f){
        if(f.path == remed.path && f.key == remed.key)
            return true;

        return false;
    };

    auto iter = std::find_if(clp.cbegin(), clp.cend(), pred);

    if(iter == clp.cend()){
        openCtree data(remed.path.filePath("symptom").toStdString());
        data.at(remed.key.toStdString(), false);
        func::repertoryData repdata(data);

        if(repdata.remedsList().isEmpty()){
            QMessageBox error;
            error.setText("Симптом без препарата!");
            error.setIcon(QMessageBox::Information);
            error.exec();
            return;
        }

        clp += remed;
        emit changeClipboardsRemed();
    }
    else{
        QMessageBox error;
        error.setText("Этот симптом уже есть в " % QString::number(clip + 1) % " клипборде!");
        error.setIcon(QMessageBox::Information);
        error.exec();
    }
}
void MainWindow::setDefaultRemedClipboardInfo(func::remedClipboardInfo & info){
    info.elim = false;
    info.cas = false;
    info.group = "";

    for(auto & it : info.measure)
        it = true;

    auto currentRep = qobject_cast<repertory*>(
        ui->mdiArea->currentSubWindow()->widget());

    info.remFilter = currentRep->getFilter();
    info.codec = currentRep->textCodec();
    info.path = currentRep->catalog();
    info.key = currentRep->currentPosition();
}
void MainWindow::clipboadrsDataChanged(){
    QAction * researchPtrs[] = {ui->clipboard1, ui->clipboard2, ui->clipboard3, ui->clipboard4,
                               ui->clipboard5, ui->clipboard6, ui->clipboard7, ui->clipboard8,
                               ui->clipboard9, ui->clipboard10};

    for(auto it = 0; it != _clipboadrs->size(); ++it)
        changeIcon(it, researchPtrs[it]->data().toBool());
}
void MainWindow::dragAndDropAddRemed(QByteArray pos, qint8 droppedClip){
    func::remedClipboardInfo info;
    info.intensity = 1;
    setDefaultRemedClipboardInfo(info);
    info.key = pos;
    addClipboardsRemed(info, droppedClip);
}
void MainWindow::addClipboardsRemedIntense1(){
    func::remedClipboardInfo info;
    info.intensity = 1;
    setDefaultRemedClipboardInfo(info);
    addClipboardsRemed(info, _defaultClipboard);
}
void MainWindow::addClipboardsRemedIntense2(){
    func::remedClipboardInfo info;
    info.intensity = 2;
    setDefaultRemedClipboardInfo(info);
    addClipboardsRemed(info, _defaultClipboard);
}
void MainWindow::addClipboardsRemedIntense3(){
    func::remedClipboardInfo info;
    info.intensity = 3;
    setDefaultRemedClipboardInfo(info);
    addClipboardsRemed(info, _defaultClipboard);
}
void MainWindow::addClipboardsRemedIntense4(){
    func::remedClipboardInfo info;
    info.intensity = 4;
    setDefaultRemedClipboardInfo(info);
    addClipboardsRemed(info, _defaultClipboard);
}
