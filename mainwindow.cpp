#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _chapters = new windowChapters(this);

    _catalog.open(QDir::toNativeSeparators("../system/catalog").toStdString());
    auto dataDirs = QDir("../data").entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    auto keysDirs = QDir("../data/keynotes").entryInfoList(QDir::Files);
    const QStringList tpCmp = {"view", "word1", "word2", "chapter", "extract", "symptom"};

    QTextCodec * cd = QTextCodec::codecForName(lang::defaultCodec());

    QStringList reperts;
    QStringList keys;
    QVector<QDir> keysPos;

    for(auto it = 0; it != _catalog.size(); ++it){
        auto str = QByteArray::fromStdString(_catalog.next());

        if(str.at(4) == 1){
            for(auto & ir : dataDirs){
                auto st = str.mid(str.lastIndexOf('\0', str.size() - 2) + 1);
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
                auto st = str.mid(str.lastIndexOf('\0', str.size() - 2) + 1);
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

    _choose = new RepChose(reperts , _repertsPos, _repertsLang, this);
    _remedList = new keysRemedList(keysPos, keys, this);
    _keych = new KeyChoose(keys, _remedList, this);

    openCtree remed(QDir("../system/remed").path().toStdString());
    openCtree author(QDir("../system/author").path().toStdString());

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

    connect(_choose, &RepChose::chooseRep, this, &MainWindow::openRepertory);
    connect(_chapters, &windowChapters::activatedBranch, this, &MainWindow::setPositionInRepertory);
    connect(ui->mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::windowChanged);
    connect(ui->action, &QAction::triggered, ui->mdiArea, &QMdiArea::cascadeSubWindows);
    connect(ui->action_3, &QAction::triggered, ui->mdiArea, &QMdiArea::tileSubWindows);
    connect(ui->action_5, &QAction::triggered, ui->mdiArea, &QMdiArea::activateNextSubWindow);
    connect(ui->action_6, &QAction::triggered, ui->mdiArea, &QMdiArea::activatePreviousSubWindow);
    connect(ui->action_4, &QAction::triggered, this, &MainWindow::openChapters);
    connect(ui->action_7, &QAction::triggered, this, &MainWindow::openChaptersInCurrentPos);
    connect(ui->action_8, &QAction::triggered, this, &MainWindow::openVocabulary);
    connect(ui->action39, &QAction::triggered, this, &MainWindow::openKeysChooseWindow);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::openRepertory(QModelIndex & item, const quint16 repLevel, std::pair<QLocale, QLocale> lang){
    auto index = item.row();
    auto rep = new repertory(QDir(_repertsPos.at(index)), QDir("../system"),
                             _cache, lang, _remedList, repLevel, ui->mdiArea);
    rep->setAttribute(Qt::WA_DeleteOnClose);
    rep->setWindowTitle(item.data().toString());
    auto repPtr = ui->mdiArea->addSubWindow(rep, Qt::SubWindow);
    repPtr->setMinimumSize(500, 500);
    repPtr->show();
}
void MainWindow::on_action1_triggered(){
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

    ui->action_4->setEnabled(!list.isEmpty());
    ui->action_7->setEnabled(!list.isEmpty());
    ui->action_8->setEnabled(!list.isEmpty());

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
