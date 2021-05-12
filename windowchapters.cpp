#include "windowchapters.h"
#include "ui_windowchapters.h"

windowChapters::windowChapters(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::windowChapters)
{
    ui->setupUi(this);
    _codec = QTextCodec::codecForName("system");
    _layout = new QStackedLayout;

    auto page1 = new QWidget(this);
    auto page2 = new QWidget(this);

    _filterModel = new QSortFilterProxyModel(this);
    _filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    _filterModel->setRecursiveFilteringEnabled(true);

    page1->setLayout(ui->verticalLayout_2);
    page2->setLayout(ui->verticalLayout_3);

    _layout->addWidget(page1);
    _layout->addWidget(page2);

    ui->groupBox->setLayout(ui->horizontalLayout_3);

    setLayout(ui->verticalLayout);

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::activated), this, &windowChapters::windowChanged);

    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(130);
    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(80);
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setItemDelegate(new delegate);
    ui->tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->verticalLayout->addLayout(_layout);

    setFixedSize(700, 700);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->buttonBox_2->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->groupBox_2->setLayout(ui->horizontalLayout_5);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &windowChapters::textFilter);
    connect(ui->lineEdit_2, &QLineEdit::textChanged, this, &windowChapters::textFilter_2);
    connect(ui->tableWidget, &QTableWidget::currentItemChanged, this, &windowChapters::selectedItemTable);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &windowChapters::accept_1);
    connect(ui->tableWidget, &QTableWidget::itemActivated, this, &windowChapters::accept_1);
    connect(ui->listView, &QListView::activated, this, &windowChapters::listClicked);
    connect(ui->pushButton, &QPushButton::clicked, this, &windowChapters::returnBranch);
    connect(ui->buttonBox_2, &QDialogButtonBox::rejected, this, &windowChapters::reject_2);
    connect(ui->buttonBox_2, &QDialogButtonBox::accepted, this, &windowChapters::sendActivatedBranch);
}

windowChapters::~windowChapters(){
    delete ui;
}

void windowChapters::getWindows(QList<QMdiSubWindow*> win, QMdiSubWindow * mdiSub){
    ui->comboBox->clear();
    _dirPaths.clear();

    for(auto it = 0; it != win.size(); ++it){
        ui->comboBox->addItem(win.at(it)->windowTitle());

        if(win.at(it) == mdiSub){
            ui->comboBox->setCurrentIndex(it);
        }

        _dirPaths.push_back(qobject_cast<repertory*>(win.at(it)->widget())->getRepDir());
    }
}
void windowChapters::selectedItemList(const QModelIndex & current){
    if(current.isValid())
        ui->buttonBox_2->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        ui->buttonBox_2->button(QDialogButtonBox::Ok)->setEnabled(false);
}
void windowChapters::windowChanged(int index){
    if(_layout->currentIndex() != 0){
        _layout->setCurrentIndex(0);
        clearModel();
    }

    ui->lineEdit->clear();
    ui->lineEdit_2->clear();

    const int mx = 8;
    QVector<QTableWidgetItem*> items;
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(mx);

    openCtree db;
    db.open(_dirPaths.at(index).filePath("symptom").toStdString());
    db.setIndex(4);

    const std::string compr(6, '\0');
    quint16 x = 0, y = 0;

    while(true){
        auto temp = db.next();

        if(x >= mx){
            ++y;
            x = 0;
        }

        if(temp.substr(12, 6) == compr){
            auto iter = std::find(temp.cbegin() + db.serviceDataLenght(), temp.cend(), '\0');
            std::string textOriginal(temp.cbegin() + db.serviceDataLenght(), iter);
            ++iter;
            std::string textLocalize(iter, std::find(iter, temp.cend(), '\0'));

            auto item_t = new QTableWidgetItem(QIcon(QPixmap(48, 48)),
                    QString::fromStdString(textOriginal + "\n") +
                    _codec->toUnicode(textLocalize.c_str()) );

            item_t->setTextAlignment(Qt::AlignHCenter);
            item_t->setData(Qt::UserRole, QByteArray::fromStdString(db.key()));
            items.push_back(item_t);
            //TODO : picture add for debug only
            ++x;
        }
        else
            break;
    }

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
}
void windowChapters::show(bool chapter){
    if(chapter)
        _layout->setCurrentIndex(0);
    else
        _layout->setCurrentIndex(1);

    if(ui->comboBox->count() != 0)
        windowChanged(ui->comboBox->currentIndex());

    if(ui->comboBox->count() == 0){
        ui->tableWidget->clear();
        clearModel();
    }

    QWidget::show();
}
void windowChapters::textFilter(const QString & text){
    auto items = ui->tableWidget->findItems(text, Qt::MatchFlag::MatchContains);

    if(!items.isEmpty())
        ui->tableWidget->setCurrentItem(items.at(0));
}
void windowChapters::textFilter_2(const QString & text){
    _filterModel->setFilterFixedString(text);//TODO : remake it
}
void windowChapters::selectedItemTable(QTableWidgetItem * item){
    if(item == nullptr || item->text().isEmpty()){
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}
void windowChapters::accept_1(){
    ui->lineEdit->clear();
    showListChapter(ui->tableWidget->currentItem()->data(Qt::UserRole).toByteArray());
    _layout->setCurrentIndex(1);
}
void windowChapters::reject_2(){
    _layout->setCurrentIndex(0);
    clearModel();
}
void windowChapters::showListChapter(const QByteArray key){
    _model = std::make_unique<searchModel>(_dirPaths.at(ui->comboBox->currentIndex()).filePath("symptom"), key);
    _filterModel->setSourceModel(_model.get());

    changeChapterText(key.right(6));
    ui->listView->setModel(_filterModel);
    ui->listView->clearFocus();
    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentChanged, this, &windowChapters::selectedItemList);
}
void windowChapters::listClicked(const QModelIndex & indexSort){
    auto index = _filterModel->mapToSource(indexSort);

    if(index.isValid()){
        ui->lineEdit_2->clear();

        auto indexPtr = static_cast<const searchModel::_node*>(index.internalPointer());
        if(indexPtr->marker()){
            if(_model->canFetchMore(index))
                _model->fetchMore(index);

            ui->listView->setRootIndex(indexSort);
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
    auto root = ui->listView->rootIndex();
    ui->listView->setRootIndex(root.parent());
    auto rootBasicModel = _filterModel->mapToSource(root);
    ui->lineEdit_2->clear();

    if(ui->listView->rootIndex() == root)
        reject_2();
    else{
        auto rootPtr = static_cast<const searchModel::_node*>(rootBasicModel.internalPointer());
        changeChapterText(rootPtr->parent()->key().right(6));
    }
}
void windowChapters::changeChapterText(const QByteArray & key){
    openCtree sym(_dirPaths.at(ui->comboBox->currentIndex()).filePath("symptom").toStdString());
    const auto column = QByteArray::fromStdString(sym.at(key.toStdString()));
    ui->label_2->setText(abstractEngine::renderingLabel(column, sym, false));
}
void windowChapters::sendActivatedBranch(){
    auto index = _filterModel->mapToSource(ui->listView->currentIndex());
    emit activatedBranch(index, ui->comboBox->currentIndex());
    clearModel();
    close();
}
void windowChapters::clearModel(){
    disconnect(ui->listView->selectionModel(), &QItemSelectionModel::currentChanged, this, &windowChapters::selectedItemList);
    _filterModel->setSourceModel(nullptr);
    _model.reset();
    ui->buttonBox_2->button(QDialogButtonBox::Ok)->setEnabled(false);
}
void windowChapters::reject(){
    clearModel();
    QDialog::reject();
}
