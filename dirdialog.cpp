#include <dirdialog.h>
#include <path.h>

ChooseDirDialog::ChooseDirDialog(QWidget *parent){
    QFile file(":/assets/dirdialog.ui");
	file.open(QIODevice::ReadOnly);

	QUiLoader loader;
	if(parent == nullptr) mainW = loader.load(&file);
	else mainW = loader.load(&file, parent);
    mainW->setWindowFlags(Qt::Window);

    dirlist = mainW->findChild<QListView*>("dirlist");
    updir   = mainW->findChild<QPushButton*>("updir");

    connect(dirlist, SIGNAL(activated(QModelIndex)), this, SLOT(enterFol(QModelIndex)));
    connect(updir, &QPushButton::clicked, this, &ChooseDirDialog::upFol);

    sm = new QFileSystemModel(this);
    dirlist->setModel(sm);
    mainW->close();
}

ChooseDirDialog::~ChooseDirDialog(){}

QDir ChooseDirDialog::getPath(){
    return QDir(QDir::homePath()); // TODO
}

QString ChooseDirDialog::getPathStr(){
    return QDir::homePath(); // TODO
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

QModelIndex ChooseDirDialog::select(const QModelIndex &in){
    QModelIndex idx = in;
    if (idx.isValid() && !dirlist->selectionModel()->isSelected(idx))
        dirlist->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    return idx;
}

QDir ChooseDirDialog::dir(){
    QString dir = Path.mcPath; // TODO: fix this shit
    return QDir(dir.isEmpty() ? QDir::homePath() : dir);
}

void ChooseDirDialog::backFol(){

}

void ChooseDirDialog::fowFol(){

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
        dirlist->edit(index);
    }
}

void ChooseDirDialog::listDC(){

}

