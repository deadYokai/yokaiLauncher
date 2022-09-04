#include <launcher.h>
#include <fstream>

#if _WIN32 || _WIN64
#define CURL_STATICLIB
#include <QWinTaskbarButton>
#endif

using namespace std;

void dp(string a){cout << a << endl;}
void dp(char a){cout << a << endl;}
void dp(int a){cout << a << endl;}
void dp(double a){cout << a << endl;}

static QWidget *loadUiFile(QString page, QWidget *parent = nullptr)
{
	QFile file(":/assets/" + page + ".ui");
	file.open(QIODevice::ReadOnly);

	QUiLoader loader;
	QWidget *k;
	if(parent == nullptr) k = loader.load(&file);
	else k = loader.load(&file, parent);
	file.close();
	return k;
}

QString importQss(){
	QFile file(":/assets/style.qss");
	file.open(QFile::ReadOnly | QFile::Text);
	QString styleSheet = QLatin1String(file.readAll());
	file.close();
	return styleSheet;
}

int importFonts(){
	QFile file(":/assets/inglobal.ttf");
	QFile file2(":/assets/inglobalb.ttf");
	file.open(QFile::ReadOnly);
	file2.open(QFile::ReadOnly);
	QFontDatabase::addApplicationFontFromData(file.readAll());
	QFontDatabase::addApplicationFontFromData(file2.readAll());
	return 1;
}

MWin::MWin(QWidget *parent) : QMainWindow(parent)
{
	ui_mw = loadUiFile("client", this);
	mm = findChild<QMainWindow*>("MainWindow");
	mwCW = findChild<QWidget*>("centralwidget");
	nickname = findChild<QLineEdit*>("nickname");
	pWidget = findChild<QWidget*>("progressWidget");
	vList = findChild<QComboBox*>("versionList");
	pLabel = findChild<QLabel*>("pLabel");
	progressBar = findChild<QProgressBar*>("progressBar");
	playBtn = findChild<QPushButton*>("playBtn");
	QMetaObject::connectSlotsByName( this );

}

MWin::~MWin(){}

void MWin::disableControls(bool a = true){
	bool val = !a;
	nickname->setEnabled(val);
	vList->setEnabled(val);
	playBtn->setEnabled(val);
}

void MWin::changeProgressState(int progress, QString text, bool showBar = true, bool show = true){
	int mH = show ? 40 : 0;
	pWidget->setMaximumHeight(mH);
	int pHeight = showBar ? 8 : 0;
	progressBar->setMaximumHeight(pHeight);
	progressBar->setValue(progress);
	pLabel->setText(text);
}

void MWin::changeProgressState(int progress, int max, QString text, bool showBar = true, bool show = true){
	int mH = show ? 40 : 0;
	pWidget->setMaximumHeight(mH);
	int pHeight = showBar ? 8 : 0;
	progressBar->setMaximumHeight(pHeight);
	progressBar->setMaximum(max);
	progressBar->setValue(progress);
	pLabel->setText(text);
}

void MWin::changeProgressState(bool show){
	int mH = show ? 40 : 0;
	pWidget->setMaximumHeight(mH);
}

QString MWin::getfilepath(QString path){
	QString ddir = QDir::cleanPath(".");
	bool useDirectory = !ddir.isEmpty() && QFileInfo(ddir).isDir();
	if(useDirectory)
		path.prepend(ddir + '/');
	return path;
}

void MWin::httpRead(){
	if(file)
		file->write(reply->readAll());
}

void MWin::getCheckSum(){
	mansha = reply->readAll();
	if(progstate == PState::MANCHEKSUM)
		manlistimport();
}

void MWin::httpFinish(){
	QFileInfo fi;
	if(progstate != PState::MANCHEKSUM){
		if (file) {
			fi.setFile(file->fileName());
			file->close();
			file.reset();
		}
		if (reply->error()) {
			QFile::remove(fi.absoluteFilePath());
			qDebug() << QString("Download failed:\n%1.").arg(reply->errorString());
			
			reply->deleteLater();
			reply = nullptr;
			return;
		}

	}
	const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

	reply->deleteLater();
	reply = nullptr;

	if (!redirectionTarget.isNull()) {
        const QUrl redirectedUrl = url.resolved(redirectionTarget.toUrl());
		if(progstate != PState::MANCHEKSUM){
			file = openFileForWrite(fi.absoluteFilePath());
			if (!file) {
				return;
			}
		}
        httpReq(redirectedUrl);
        return;
    }

}

std::unique_ptr<QFile> MWin::openFileForWrite(const QString &fileName){
	std::unique_ptr<QFile> file(new QFile(fileName));
	if (!file->open(QIODevice::WriteOnly)) {
        qDebug() << QString("Unable to save the file %1: %2.").arg(QDir::toNativeSeparators(fileName), file->errorString());
        return nullptr;
    }
    return file;

}

void MWin::progress_func(qint64 bytesRead, qint64 totalBytes)
{
	changeProgressState((int)bytesRead, (int)totalBytes, QString::fromStdString((int)bytesRead+"/"+(int)totalBytes));
}


void MWin::manlistimport(){
	progstate = PState::INIT;
	QFile f(getfilepath("yokaiLauncher_manifest.json"));
	if (!f.open(QFile::ReadOnly | QFile::Text)) return;
	QTextStream in(&f);
	QString str = in.readAll();
	QCryptographicHash hash(QCryptographicHash::Sha256);
	QByteArray header = str.toUtf8();
	hash.addData(header.data());
	QString a = hash.result().toHex();
	qDebug() << "Checksum: " << mansha;
	if(a != mansha){
		progstate = PState::MANCHEKSUM;
		progressFinish();
		return;
	}
	QJsonDocument jsonResponse = QJsonDocument::fromJson(str.toUtf8());
	QJsonObject jsonObject = jsonResponse.object();
	QString latestVersion = jsonObject["latest"].toObject()["release"].toString();
	QJsonArray vData = jsonObject["versions"].toArray();
	qDebug() << "Latest version: " << latestVersion;
	for (QJsonArray::iterator it = vData.begin(); it != vData.end(); ++it) {
		QJsonValue a = *it;
		QJsonObject jo = a.toObject();
		if(jo["type"].toString() == "release"){
			vList->addItem(jo["id"].toString());
		}
	}
	vList->setCurrentIndex(0);
	changeProgressState(false);
	disableControls(false);
}

void MWin::progressFinish(){
	switch(progstate){
		case PState::MANDOWN:
			manlistimport();
			break;
		case PState::MANCHEKSUM:
			changeProgressState(0, "Checking manifest checksum...", false);
			qDebug() << "Checking manifest checksum...";
			httpReq(QUrl("https://vilafox.xyz/api/yokaiLauncher?get=sha"));
			break;
		default:
			changeProgressState(0, "Done.", false);
			disableControls(false);
	}
}

void MWin::httpReq(const QUrl &requestedUrl) {
	reply = qnam.get(QNetworkRequest(requestedUrl));
	connect(reply, &QNetworkReply::finished, this, &MWin::httpFinish);
	if(progstate != PState::MANCHEKSUM){
		connect(reply, &QIODevice::readyRead, this, &MWin::httpRead);
		connect(reply, &QNetworkReply::downloadProgress, this, &MWin::progress_func);
		connect(reply, &QNetworkReply::finished, this, &MWin::progressFinish);
	}else{
		connect(reply, &QIODevice::readyRead, this, &MWin::getCheckSum);
	}
}

void MWin::downloadFile(const QUrl &requestedUrl, QString path){
	
	path = getfilepath(path);

	if(QFile::exists(path))
		QFile::remove(path);

	file = openFileForWrite(path);
	if(!file)
		return;
	httpReq(requestedUrl);
}

void MWin::purl(){
	changeProgressState(0, "Getting version manifest...", false);
	QString manpath = "yokaiLauncher_manifest.json";
	dp("Getting version manifest...");
	progstate = PState::MANDOWN;
	if(!QFile::exists(getfilepath(manpath)))
		downloadFile(QUrl("https://vilafox.xyz/api/yokaiLauncher_manifest.json"), manpath);
	else
		progressFinish();

}

void MWin::on_playBtn_clicked(){
	changeProgressState(100, "Play button clicked");
	dp("Play button clicked");
}

void MWin::appshow(){
	ui_mw->show();
	dp("UI loaded");
#if _WIN32 || _WIN64
	dp("Win show taskbar icon");
	QWinTaskbarButton *button = new QWinTaskbarButton(this);
    button->setWindow(windowHandle());
    button->setOverlayIcon(QIcon(":/assets/icon.svg"));
#endif
	disableControls();
	// auto future = QtConcurrent::run(mem_fn(&MWin::st), this);
	purl();
	
}

int main(int argc, char *argv[])
{
	dp("Init");
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication app(argc, argv);
	app.setStyleSheet(importQss());
	app.setWindowIcon(QIcon(":/assets/icon.svg"));
	// importFonts();
	MWin n;
	n.progstate = PState::INIT;
	n.appshow();
	return app.exec();
}
