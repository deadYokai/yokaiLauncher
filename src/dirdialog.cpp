#include <dirdialog.h>
#include <path.h>

ChooseDirDialog::ChooseDirDialog(QWidget *parent){
    QFile file(":/assets/dirdialog.ui");
	file.open(QIODevice::ReadOnly);

	QUiLoader loader;
	if(parent == nullptr) mainW = loader.load(&file);
	else mainW = loader.load(&file, parent);
    mainW->setWindowFlags(Qt::Window);

    dirlist  = mainW->findChild<QListView*>("dirlist");
    updir    = mainW->findChild<QPushButton*>("updir");
    newDirB  = mainW->findChild<QPushButton*>("newDirB");
    choose   = mainW->findChild<QPushButton*>("choose");
    currPath = mainW->findChild<QLineEdit*>("currPath");
    backdir  = mainW->findChild<QPushButton*>("backdir");
    fowdir   = mainW->findChild<QPushButton*>("fowdir");

    connect(dirlist, SIGNAL(activated(QModelIndex)), this, SLOT(enterFol(QModelIndex)));
    connect(updir, &QPushButton::clicked, this, &ChooseDirDialog::upFol);
    connect(newDirB, &QPushButton::clicked, this, &ChooseDirDialog::newDir);
    connect(fowdir, &QPushButton::clicked, this, &ChooseDirDialog::fowFol);
    connect(backdir, &QPushButton::clicked, this, &ChooseDirDialog::backFol);
    // connect(choose, &QPushButton::clicked, this, &ChooseDirDialog::select);

    sm = new QFileSystemModel(mainW);

    connect(sm, SIGNAL(fileRenamed(QString, QString, QString)), this, SLOT(frm(QString, QString, QString)));
    connect(sm, SIGNAL(rootPathChanged(QString)), this, SLOT(pch(QString)));

    sm->setFilter(QDir::NoDot | QDir::Dirs | QDir::Hidden);
    dirlist->setModel(sm);
    mainW->close();
}

ChooseDirDialog::~ChooseDirDialog(){}

void ChooseDirDialog::pch(QString newPath){
    const QString newNativePath = QDir::toNativeSeparators(newPath);

    if(currHistoryLoc < 0 || currHistory.value(currHistoryLoc).path != newNativePath){
        if(currHistoryLoc >= 0) saveHisSel();
        while(currHistoryLoc >= 0 && currHistoryLoc + 1 < currHistory.size()){
            currHistory.removeLast();
        }
        qDebug() << "1";
        currHistory.append({newNativePath, PersistentModelIndexList()});
        ++currHistoryLoc;
    }
}

void ChooseDirDialog::frm(const QString &path, const QString &oldName, const QString &newName){
    if(currPath->text() == oldName){
        currPath->setText(newName);
    }
}

QDir ChooseDirDialog::getPath(){
    return QDir(pathStr);
}

QString ChooseDirDialog::getPathStr(){
    return pathStr;
}

QModelIndex ChooseDirDialog::rootIn(){
    return dirlist->rootIndex();
}

void ChooseDirDialog::setDir(const QString &path){
    QString newdir = path;
    if(!path.isEmpty()) newdir = QDir::cleanPath(path);
    if(!path.isEmpty() && newdir.isEmpty()) return;
    QModelIndex root = sm->setRootPath(newdir);
    dirlist->setRootIndex(root);
}

void ChooseDirDialog::open(QString startpos){
    setDir(startpos);
    mainW->show();
}

void ChooseDirDialog::enterFol(const QModelIndex &index){
    QString path = index.data(QFileSystemModel::FilePathRole).toString();
    if(path.isEmpty() || sm->isDir(index)){
        setDir(path);
    }
}

void ChooseDirDialog::upFol(){
    QDir dir(sm->rootDirectory());
    QString newDirectory;
    if (dir.isRoot()) {
        newDirectory = sm->myComputer().toString();
    } else {
        dir.cdUp();
        newDirectory = dir.absolutePath();
    }
    setDir(newDirectory);
}

QModelIndex ChooseDirDialog::select(const QModelIndex &idx){
    
    if (idx.isValid() && !dirlist->selectionModel()->isSelected(idx))
        dirlist->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    return idx;
}

QDir ChooseDirDialog::dir(){
    QString dir = Path.mcPath;
    return QDir(dir.isEmpty() ? QDir::homePath() : dir);
}


void ChooseDirDialog::navigate(HistoryItem his){
    setDir(his.path);
    qDebug() << his.path;
    if(his.selection.isEmpty()) return;

    if(std::any_of(his.selection.cbegin(), his.selection.cend(), [](const QPersistentModelIndex &i){ return !i.isValid(); })){
        his.selection.clear();
        return;
    }

    QAbstractItemView *view = static_cast<QAbstractItemView*>(dirlist);
    auto selmodel = view->selectionModel();
    const QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Select | QItemSelectionModel::Rows;

    selmodel->select(his.selection.constFirst(), flags | QItemSelectionModel::Clear | QItemSelectionModel::Current);

    for(int i = 0, size = his.selection.size(); i < size; i++)
        selmodel->select(his.selection.at(i), flags);
    
    view->scrollTo(his.selection.constFirst());

}

void ChooseDirDialog::saveHisSel(){
    if(currHistoryLoc < 0 || currHistoryLoc >= currHistory.size()) return;

    auto &item = currHistory[currHistoryLoc];
    item.selection.clear();

    const auto selIndexes = dirlist->selectionModel()->selectedRows();
    for(const auto &index : selIndexes)
        item.selection.append(QPersistentModelIndex(index));
}

void ChooseDirDialog::backFol(){
    if(!currHistory.isEmpty() && currHistoryLoc > 0){
        saveHisSel();
        navigate(currHistory[--currHistoryLoc]);
    }
}

void ChooseDirDialog::fowFol(){
    if(!currHistory.isEmpty() && currHistoryLoc < currHistory.size() - 1){
        saveHisSel();
        navigate(currHistory[++currHistoryLoc]);
    }
}

void ChooseDirDialog::newDir(){
    dirlist->clearSelection();

    QString fname = "NewDir";
    QString prefix = dir().absolutePath() + QDir::separator();
    if(QFile::exists(prefix + fname)){
        qlonglong suf = 2;
        while(QFile::exists(prefix + fname)){
            fname = "NewDir" + QString::number(suf++);
        }
    }

    QModelIndex par = rootIn();
    QModelIndex index = sm->mkdir(par, fname);
    if(!index.isValid()) return;

    index = select(index);
    
    if(index.isValid()){
        dirlist->setCurrentIndex(index);
        dirlist->edit(index);
    }
}

void ChooseDirDialog::listDC(){

}

