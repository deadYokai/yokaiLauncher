#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QProgressBar>
#include <QApplication>
#include <QFile>
#include <QUiLoader>
#include <QFontDatabase>
#include <QScreen>
#include <QObject>
#include <QVBoxLayout>
#include <QShowEvent> 
#include <QThread>
#include <QFuture> 
#include <QtConcurrent/QtConcurrentRun>
#include <QStringList> 
#include <iostream>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QTextStream> 
#include <QJsonObject> 
#include <QCryptographicHash> 
#include <config_manager.h>

QT_BEGIN_NAMESPACE
class QFile;
class CMan;
class QNetworkReply;
QT_END_NAMESPACE

using namespace std;

enum class PState {
    INIT,
    MANDOWN,
    MANCHEKSUM,
    VERMANDOWN,
    ASSDOWN,
    LIBDOWN,
    READY2PLAY
};

class MWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit MWin(QWidget *parent = nullptr);
    ~MWin();
    void appshow();
    void disableControls(bool);
    void changeProgressState(int, QString, bool, bool);
    void changeProgressState(int, int, QString, bool, bool);
    void changeProgressState(bool);
    QComboBox *vList;
    void purl();
    void httpReq(const QUrl &requestedUrl);
    void downloadFile(const QUrl &requestedUrl, QString path);
    void progress_func(qint64 bytesRead, qint64 totalBytes);
    void st();
    void httpFinish();
    void httpRead();
    void manlistimport();
    void progressFinish();
    std::unique_ptr<QFile> openFileForWrite(const QString &fileName);
    QString getfilepath(QString path);
    PState progstate;
    QString mansha;
    void getCheckSum();    
    QMap<QString, QString> vData;
    void loadconf();
    void vermandown();
    void libdown();
    void assdown();
    QString currFile = nullptr;
    QString currManFile = nullptr;
    QJsonObject currmanj;
    QJsonObject assmanj;
    int ass = 0;
    qsizetype assm = 0;
    CMan *config;
    QStringList getJargs();
    void launch();
    bool checkJava();

private slots:
    void on_playBtn_clicked();
    void verChanged(const QString &text);
    void mcend(int exitCode, QProcess::ExitStatus ExitStatus);


private:
    QWidget *ui_mw;
    QPushButton *playBtn;
    QWidget *mwCW;
    QMainWindow *mm;
    QLineEdit *nickname;
    QLabel *cNick;
    QLabel *pLabel;
    QWidget *pWidget;
    QProgressBar *progressBar;
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    QUrl url;
    std::unique_ptr<QFile> file;
    QProcess *process;
    bool run = false;
};
