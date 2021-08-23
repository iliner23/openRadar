#include "windowchapters.h"
#include "ui_windowchapters.h"

windowChapters::windowChapters(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::windowChapters)
{
    ui->setupUi(this);
    _model = new searchModel(this);
    _layout = new QStackedLayout;

    auto page1 = new QWidget(this);
    auto page2 = new QWidget(this);

    _filterModel = new proxySearchModel(this);
    _filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    _filterModel->setRecursiveFilteringEnabled(false);
    _filterModel->setSourceModel(_model);

    ui->listView->setModel(_filterModel);

    page1->setLayout(ui->verticalLayout_2);
    page2->setLayout(ui->verticalLayout_3);

    _layout->addWidget(page1);
    _layout->addWidget(page2);

    ui->groupBox->setLayout(ui->horizontalLayout_3);

    setLayout(ui->verticalLayout);

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::activated), this, &windowChapters::setActiveRepertory);

    ui->tableWidget->setItemDelegate(new delegate);
    ui->verticalLayout->addLayout(_layout);

    setFixedSize(680, 700);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->buttonBox_2->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->groupBox_2->setLayout(ui->horizontalLayout_5);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &windowChapters::textFilter);
    connect(ui->lineEdit_2, &QLineEdit::textChanged, this, &windowChapters::textFilter_2);

    connect(ui->tableWidget, &QTableWidget::currentItemChanged, this, &windowChapters::selectedItemTable);
    connect(ui->tableWidget, &QTableWidget::itemActivated, this, &windowChapters::accept_1);

    connect(ui->listView, &QListView::clicked, this, &windowChapters::listClicked);
    connect(ui->listView, &QListView::activated, this, &windowChapters::listClicked);

    connect(ui->pushButton, &QPushButton::clicked, this, &windowChapters::returnBranch);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &windowChapters::accept_1);
    connect(ui->buttonBox_2, &QDialogButtonBox::rejected, this, &windowChapters::reject_2);
    connect(ui->buttonBox_2, &QDialogButtonBox::accepted, this, &windowChapters::sendActivatedBranch);

    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentChanged, this, &windowChapters::selectedItemList);
}

windowChapters::~windowChapters(){
    delete ui;
}
void windowChapters::selectedItemList(const QModelIndex & current){
    if(current.isValid())
        ui->buttonBox_2->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        ui->buttonBox_2->button(QDialogButtonBox::Ok)->setEnabled(false);
}
void windowChapters::setActiveRepertory(int comboIndex){
    if(_layout->currentIndex() != 0){
        _layout->setCurrentIndex(0);
        setWindowTitle("Окно выбора глав");
        clearModel();
    }

    tableRender(comboIndex);
}
void windowChapters::tableRender(int comboIndex){
    ui->lineEdit->clear();
    ui->lineEdit_2->clear();
    _codec = ui->comboBox->itemData(comboIndex).value<repertory*>()->textCodec();

    constexpr int mx = 8;
    QVector<QTableWidgetItem*> items;
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(mx);

    openCtree db;
    db.open(_dirPaths.at(comboIndex).filePath("symptom").toStdString());
    db.setIndex(4);

    quint16 x = 0, y = 0;

    while(true){
        auto temp = QByteArray::fromStdString(db.next());

        if(x >= mx){
            ++y;
            x = 0;
        }

        if(temp.mid(12, 6) == QByteArray(6, '\0')){
            auto return_size = [](const auto & value, const auto & _size){
                return (value == -1) ? _size : value;
            };

            auto iter = return_size(temp.indexOf('\0', db.serviceDataLenght()), temp.size());
            const auto textOriginal = temp.mid(db.serviceDataLenght(), iter - db.serviceDataLenght());
            ++iter;
            const auto textLocalize = temp.mid(iter, return_size(temp.indexOf('\0', iter) - iter, temp.size()));

            auto item_t = new QTableWidgetItem(QIcon(QPixmap(48, 48)),
                        _codec->toUnicode(textOriginal % "\n") +
                        _codec->toUnicode(textLocalize));

            item_t->setTextAlignment(Qt::AlignHCenter);
            item_t->setData(Qt::UserRole, QByteArray::fromStdString(db.key()));
            items.push_back(item_t);
            //TODO : picture add for debug only
            ++x;
        }
        else
            break;
    }

    ui->tableWidget->setUpdatesEnabled(false);

    if(y == 0 && !items.empty())
        ui->tableWidget->setRowCount(1);
    else
        ui->tableWidget->setRowCount(y + 1);

    x = 0; y = 0;

    for(auto & it : items){
        if(x >= mx){
            ++y;
            x = 0;
        }

        ui->tableWidget->setItem(y, x, it);
        ++x;
    }

    for(; x != mx; ++x){
        auto emptyItem = new QTableWidgetItem();
        auto fl = emptyItem->flags();
        fl.setFlag(Qt::ItemIsEnabled, false);
        emptyItem->setFlags(fl);
        ui->tableWidget->setItem(y, x, emptyItem);
    }

    ui->tableWidget->setUpdatesEnabled(true);
}
void windowChapters::show(QList<QMdiSubWindow*> win, QMdiSubWindow * mdiSub){
    ui->comboBox->clear();
    _dirPaths.clear();
    setWindowTitle("Окно выбора глав");

    for(auto it = 0; it != win.size(); ++it){
        ui->comboBox->addItem(win.at(it)->windowTitle(),
            QVariant::fromValue(win.at(it)->widget()));

        if(win.at(it) == mdiSub){
            ui->comboBox->setCurrentIndex(it);
        }

        _dirPaths.push_back(qobject_cast<repertory*>
                            (win.at(it)->widget())->catalog());
    }

    if(ui->comboBox->count() != 0)
        tableRender(ui->comboBox->currentIndex());
    else
        ui->tableWidget->clear();

    _layout->setCurrentIndex(0);
    QWidget::show();
}
void windowChapters::show(QList<QMdiSubWindow*> win, QMdiSubWindow * mdiSub, const QByteArray key){
    setWindowTitle("Окно выбора симптома");
    ui->comboBox->clear();
    _dirPaths.clear();

    for(auto it = 0; it != win.size(); ++it){
        ui->comboBox->addItem(win.at(it)->windowTitle(),
                QVariant::fromValue(win.at(it)->widget()));

        if(win.at(it) == mdiSub)
            ui->comboBox->setCurrentIndex(it);

        _dirPaths.push_back(qobject_cast<repertory*>
                            (win.at(it)->widget())->catalog());
    }

    if(ui->comboBox->count() != 0)
        tableRender(ui->comboBox->currentIndex());
    else
        ui->tableWidget->clear();

    openCtree db;
    db.open(_dirPaths.at(ui->comboBox->currentIndex()).filePath("symptom").toStdString());
    db.at(key.toStdString(), false);

    auto path = func::getRootPath(db);
    showListChapter(QByteArray(6, '\0') + path.back());
    auto iterPath = (path.size() > 1) ? path.cbegin() + 1 : path.cbegin();
    changeChapterText(*iterPath);
    path.pop_back();

    for(auto it = path.crbegin(); it != path.crend(); ++it){
        const auto baseKey =
                _filterModel->mapFromSource(
                    _model->keyToIndex(*it,
                    _filterModel->mapToSource(ui->listView->rootIndex())));

        if(it != path.crend() - 1){
            _filterModel->setRootIndex(baseKey);
            ui->listView->setRootIndex(baseKey);
        }
        else{
            ui->listView->scrollTo(baseKey);
            ui->listView->setCurrentIndex(baseKey);
        }
    }

    const auto ptr = static_cast<const searchModel::_node*>
            (_filterModel->mapToSource(ui->listView->currentIndex()).internalPointer());

    if(ptr != nullptr)
        changeChapterText(ptr->parent()->key().right(6));

    _layout->setCurrentIndex(1);
    QWidget::show();
}
void windowChapters::textFilter(const QString text){
    auto items = ui->tableWidget->findItems(text, Qt::MatchFlag::MatchContains);

    if(!items.isEmpty())
        ui->tableWidget->setCurrentItem(items.at(0));
}
void windowChapters::textFilter_2(const QString text){
    _filterModel->setFilterFixedString(text);
}
void windowChapters::selectedItemTable(QTableWidgetItem * item){
    if(item == nullptr || item->text().isEmpty())
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}
void windowChapters::accept_1(){
    const auto key = ui->tableWidget->currentItem()->data(Qt::UserRole).toByteArray();
    ui->lineEdit->clear();
    showListChapter(key);
    changeChapterText(key.right(6));
    setWindowTitle("Окно выбора симптома");
    _layout->setCurrentIndex(1);
}
void windowChapters::reject_2(){
    _layout->setCurrentIndex(0);
    setWindowTitle("Окно выбора глав");
    clearModel();
}
void windowChapters::showListChapter(const QByteArray key){
    _model->setCatalogFile(_dirPaths.at(ui->comboBox->currentIndex()).filePath("symptom"), key, _codec);
    _filterModel->setSourceModel(_model);
    ui->listView->clearFocus();
}
void windowChapters::listClicked(const QModelIndex & indexSort){
    if(indexSort.isValid()){
        auto index = _filterModel->mapToSource(indexSort);
        ui->lineEdit_2->clear();
        auto indexPtr = static_cast<const searchModel::_node*>(index.internalPointer());

        if(indexPtr->marker()){
            _filterModel->setRootIndex(_filterModel->mapFromSource(index));
            ui->listView->setRootIndex(_filterModel->mapFromSource(index));

            changeChapterText(indexPtr->key().right(6));
        }
        else{
            emit activatedBranch(index, ui->comboBox->currentIndex());
            clearModel();
            close();
        }
    }
}
void windowChapters::returnBranch(){
    ui->lineEdit_2->clear();
    auto root = ui->listView->rootIndex();

    _filterModel->setRootIndex(root.parent());
    ui->listView->setRootIndex(root.parent());
    ui->listView->scrollTo(root);

    auto rootBasicModel = _filterModel->mapToSource(root);

    if(ui->listView->rootIndex() == root)
        reject_2();
    else{
        auto rootPtr = static_cast<const searchModel::_node*>(rootBasicModel.internalPointer());
        changeChapterText(rootPtr->parent()->key().right(6));
    }
}
void windowChapters::changeChapterText(const QByteArray key){
    openCtree sym(_dirPaths.at(ui->comboBox->currentIndex()).filePath("symptom").toStdString());
    sym.at(key.toStdString(), false);
    ui->label_2->setText(func::renderingLabel(sym, false, _codec));
}
void windowChapters::sendActivatedBranch(){
    QModelIndex index;

    if(ui->listView->currentIndex().isValid())
        index = _filterModel->mapToSource(ui->listView->currentIndex());
    else
        index = _filterModel->mapToSource(ui->listView->rootIndex());

    emit activatedBranch(index, ui->comboBox->currentIndex());
    clearModel();
    close();
}
void windowChapters::clearModel(){
    _filterModel->setRootIndex(QModelIndex());
    ui->buttonBox_2->button(QDialogButtonBox::Ok)->setEnabled(false);
}
void windowChapters::reject(){
    clearModel();
    QDialog::reject();
}
