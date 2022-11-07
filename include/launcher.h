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
// #include <iostream>
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

#include <classes.h>

QT_BEGIN_NAMESPACE
class QFile;
class CMan;
class QNetworkReply;
QT_END_NAMESPACE

class MWin : public QMainWindow, Ui::MWin
{
    // Q_OBJECT

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
    void downloadFile(const QUrl &requestedUrl, QString path); //to remove
    void progress_func(qint64 bytesRead, qint64 totalBytes); //update
    void st();
    void httpFinish(); //to remove
    void httpRead(); //to remove
    void manlistimport();
    void progressFinish(); //to remove
    std::unique_ptr<QFile> openFileForWrite(const QString &fileName); //to remove
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
    QString quiltVer;
    QString MLMclass;
    QString MLLibs;
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
    QList<MLMaven*> dwList;

    void doDownload(const QUrl &url, const QString &path);
    void dwF(QList<MLMaven*> files);
    QString Fpath;
    bool saveToDisk(const QString &filename, QIODevice *data);
    bool isHttpRedirect(QNetworkReply *reply);

    void r2run();
    void quiltpost();
    void fabpost();

    void quiltDownload();
    ModLoader cML;
    QList<MLMaven*> getMavenData(QJsonArray ja);
    

public slots:
    void downloadFinished(QNetworkReply *reply);
    void sslErrors(const QList<QSslError> &errors);
    void verChanged(const QString &text);
    void mcend(int exitCode, QProcess::ExitStatus ExitStatus);
    void mlChanged(bool state);
    void rr();
    void re();
    void settbtn_click();
    void pageBtnClick();

private:
    void closeEvent(QCloseEvent *bar);
    Ui::Form *dcon;
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
    QNetworkAccessManager qnam; //to remove
    QNetworkReply *reply; //to remove
    QUrl url;
    std::unique_ptr<QFile> file; //to remove
    QProcess *process;
    bool run = false;
    Ui::MWin *ui;
    QWidget *settingsWidget;
    QPushButton *settsavebtn;
    QComboBox *themeBox;
    QPushButton *mcFolBtn;
    QLineEdit *mcPathEdit;
    QPushButton *mcPathSel;
    QList<QNetworkReply*> dwFiles; //to remove

    QNetworkAccessManager manager;
    QList<QNetworkReply *> currentDownloads;
    QString filepath;
    QMap<QNetworkReply*, QString> dwMap;
    
    QCheckBox *isf;
	QLineEdit *jvma;
	QSlider *rams;
	QLabel *mml;
	QLabel *cram;
    bool m = false; //to remove
};

