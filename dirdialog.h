#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QDir>
#include <QUiLoader>
#include <QFileSystemModel>
#include <QModelIndex>

class QAbstractProxyModel;

class ChooseDirDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ChooseDirDialog(QWidget *parent = nullptr);
    ~ChooseDirDialog();
    void open(QString startpos = QDir::homePath());
    QDir getPath();
    QString getPathStr();

private slots:
    void upFol();
    void backFol();
    void fowFol();
    void newDir();
    void listDC();
    void enterFol(const QModelIndex &index);

private:
    QWidget*          mainW;
    QString           pathstr;
    QPushButton*      choose;
    QPushButton*      cancel;
    QPushButton*      updir;
    QPushButton*      backdir;
    QPushButton*      fowdir;
    QPushButton*      cdir;
    QPushButton*      newDirB;
    QListView*        dirlist;
    QLineEdit*        path;
    QLineEdit*        currPath;
    QLineEdit*        newDirE;
    QLabel*           dirPathL;
    QLabel*           currPathL;
    QFileSystemModel* sm;

    QDir        dir();
    QModelIndex select(const QModelIndex &in);
    QModelIndex rootIn();
    void        setDir(const QString &path);
};
