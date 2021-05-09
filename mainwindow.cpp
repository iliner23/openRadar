#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::windowChanged);
    connect(ui->action, &QAction::triggered, ui->mdiArea, &QMdiArea::cascadeSubWindows);
    connect(ui->action_3, &QAction::triggered, ui->mdiArea, &QMdiArea::tileSubWindows);
    connect(ui->action_5, &QAction::triggered, ui->mdiArea, &QMdiArea::activateNextSubWindow);
    connect(ui->action_6, &QAction::triggered, ui->mdiArea, &QMdiArea::activatePreviousSubWindow);
    connect(ui->action_4, &QAction::triggered, this, &MainWindow::openChapters);

    _chapters = new windowChapters(this);
    connect(_chapters, &windowChapters::activatedBranch, this, &MainWindow::setPositionInRepertory);

    _catalog.open(QDir::toNativeSeparators("../system/catalog").toStdString());
    auto dataDirs = QDir("../data").entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    auto keysDirs = QDir("../data/keynotes").entryInfoList(QDir::Files);
    const QStringList tpCmp = {"view", "word1", "word2", "chapter", "extract", "symptom"};

    for(auto it = 0; it != _catalog.size(); ++it){
        auto str = _catalog.next();

        if(str.at(4) == 1){
            for(auto & ir : dataDirs){
                QString dir;
                auto st = str.substr(str.rfind('\0', str.size() - 2) + 1);
                st.erase(st.end() - 2, st.end());
                dir = QDir::toNativeSeparators(QString::fromStdString(st));

                if(dir.compare(QDir::toNativeSeparators(ir.filePath()), Qt::CaseInsensitive) == 0){
                    const auto reclen = _catalog.serviceDataLenght();
                    const auto tp = str.find('\0', reclen);
                    bool exit = false;

                    for(auto & iqcmp : tpCmp){
                        if(!QDir(dir).exists(iqcmp + ".dat")){
                            exit = true;
                            break;
                        }
                    }

                    if(exit)
                        continue;

                    _reperts.append(QString::fromStdString(str.substr(tp + 1, str.find('\0', tp + 1) - tp - 1)));
                    _reperts.append(QString::fromStdString(str.substr(reclen, tp - reclen)));
                    _reperts.append(dir);
                }
            }
        }
        else if(str.at(4) == 2){
            for(auto & ir : keysDirs){
                QString dir;
                auto st = str.substr(str.rfind('\0', str.size() - 2) + 1);
                st.erase(st.end() - 1, st.end());
                dir = QDir::toNativeSeparators(QString::fromStdString(st));

                if(QString::compare(dir + ".dat", QDir::toNativeSeparators(ir.filePath()), Qt::CaseInsensitive) == 0){
                    const auto reclen = _catalog.serviceDataLenght();
                    const auto tp = str.find('\0', reclen);
                    _keys.append(QString::fromStdString(str.substr(tp + 1, str.find('\0', tp + 1) - tp - 1)));
                    _keys.append(QString::fromStdString(str.substr(reclen, tp - reclen)));
                    _keys.append(dir);
                }
            }
        }
        else
            break;
    }

    _choose = new RepChose(_reperts, this);
    connect(_choose, &RepChose::chooseRep, this, &MainWindow::openRepertory);

    openCtree remed(QDir::toNativeSeparators("../system/remed").toStdString());
    openCtree author(QDir::toNativeSeparators("../system/author").toStdString());
    _cache._lenRem = remed.serviceDataLenght();
    _cache._lenAuthor = author.serviceDataLenght();

    _cache._cacheRemed.reserve(remed.size());
    _cache._cacheAuthor.reserve(author.size());

    for(auto it = 0; it != remed.size(); ++it){
        auto tp = remed.next();
        auto key = remed.key();
        quint16 kt;
        ((char *)&kt)[0] = key[1];
        ((char *)&kt)[1] = key[0];
        _cache._cacheRemed[kt] = std::move(tp);
    }

    for(auto it = 0; it != author.size(); ++it){
        auto tp = author.next();
        auto key = author.key();
        quint16 kt;
        ((char *)&kt)[0] = key[1];
        ((char *)&kt)[1] = key[0];
        _cache._cacheAuthor[kt] = std::move(tp);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::openRepertory(QListWidgetItem * item, const quint16 repLevel){
    auto index = item->listWidget()->row(item);
    auto rep = new repertory(QDir(_reperts.at(index * 3 + 2)), QDir("../system"), _cache, repLevel, ui->mdiArea);
    rep->setAttribute(Qt::WA_DeleteOnClose);
    rep->setWindowTitle(item->text());
    auto repPtr = ui->mdiArea->addSubWindow(rep);
    repPtr->setMinimumSize(500, 500);
    repPtr->show();
}
void MainWindow::on_action1_triggered()
{
    _choose->show();
}
void MainWindow::windowChanged(){
    auto act = ui->menu_6->actions();
    const auto size = act.size();

    for(auto i = 6; i < size; ++i){
        ui->menu_6->removeAction(act.at(i));
    }

    const auto sub = ui->mdiArea->activeSubWindow();
    auto list = ui->mdiArea->subWindowList();

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
    _chapters->getWindows(ui->mdiArea->subWindowList(), ui->mdiArea->activeSubWindow());
    _chapters->show();
}
void MainWindow::setPositionInRepertory(const QModelIndex & pos, const qint32 winIndex){
    auto list = ui->mdiArea->subWindowList();
    if(!list.isEmpty() && winIndex < list.size()){
        auto window = qobject_cast<repertory*>(list.at(winIndex)->widget());
        window->setPosition(static_cast<const searchModel::_node *>(pos.internalPointer())->key().right(6));
    }
}
