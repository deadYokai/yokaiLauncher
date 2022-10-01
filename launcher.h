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
#include <QCheckBox>
#include <QPlainTextEdit> 
#include <QSettings> 
#include <QResizeEvent> 
#include <QSlider>
#include <config_manager.h>
#include <QAbstractItemView>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QPainter>

QT_BEGIN_NAMESPACE
class QFile;
class CMan;
class QNetworkReply;
QT_END_NAMESPACE


enum class PState {
    INIT,
    MANDOWN,
    MANCHEKSUM,
    VERMANDOWN,
    ASSDOWN,
    LIBDOWN,
    READY2PLAY,
    FabricDown,
    JAVAIN
};

class MWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit MWin(QWidget *parent = nullptr);
    ~MWin();
    void appshow();
    QString importStyle(QString stylePath);
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
    qsizetype ass = 0;
    qsizetype assm = 0;
    CMan *config;
    QString fabVer;
    QString fabMclass;
    QString fabLibs;
    QStringList getJargs();
    void launch();
    void checkJava();
    bool isWhiteSpace(const QString & str);
    void fabricDownload();
    bool debug;
    QString javaRuntimeDir;
    void msgBox(QString msg);
    bool offline = false;
    QWidget *ui_mw;
    QPixmap _pixmapBg;
    void paintEvent(QPaintEvent *pe);
    bool checksha1(QString, QString);

private slots:
    void verChanged(const QString &text);
    void mcend(int exitCode, QProcess::ExitStatus ExitStatus);
    void isFabricbox(int state);
    void rr();
    void re();
    void settbtn_click();

private:
    void closeEvent(QCloseEvent *bar);
    QWidget *dcon;
    QWidget *bwi;
    QWidget *uwi;
    QPushButton *playBtn;
    QPushButton *settingsb;
    QCheckBox *fabricb;
    QWidget *mwCW;
    QWidget *mm;
    QLineEdit *nickname;
    QLabel *cNick;
    QLabel *pLabel;
    QLabel *bid;
    QWidget *pWidget;
    QProgressBar *progressBar;
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    QUrl url;
    std::unique_ptr<QFile> file;
    QProcess *process;
    bool run = false;

    QWidget *settingsWidget;
    QPushButton *settsavebtn;
    QComboBox *themeBox;
    QPushButton *mcFolBtn;
    QLineEdit *mcPathEdit;
    QPushButton *mcPathSel;
};
