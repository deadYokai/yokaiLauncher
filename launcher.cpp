#include <launcher.h>
#include <fstream>

#ifdef Q_OS_WIN
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
	QString ddir = QDir::cleanPath(mcPath);
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
	if(assm != 0){
		changeProgressState((int)ass, static_cast<int>(assm), "Downloading assets " + QString::number(ass) + "/" + QString::number(static_cast<int>(assm)));
	}else{
		if(currFile != nullptr){
			QFileInfo fi(currFile);
			changeProgressState((int)bytesRead, (int)totalBytes, fi.fileName());
		}
		else
			changeProgressState((int)bytesRead, (int)totalBytes, QString::fromStdString((int)bytesRead+"/"+(int)totalBytes));
	}
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
	
	QJsonArray vDataArr = jsonObject["versions"].toArray();
	qDebug() << "Latest version: " << latestVersion;
	for (QJsonArray::iterator it = vDataArr.begin(); it != vDataArr.end(); ++it) {
		QJsonValue b = *it;
		QJsonObject jo = b.toObject();
		if(jo["type"].toString() == "release"){
			vData.insert(jo["id"].toString(), jo["url"].toString());
			vList->addItem(jo["id"].toString());
		}
	}
	vList->setCurrentIndex(0);
	changeProgressState(false);
	disableControls(false);
}

void MWin::assdown(){
	QJsonObject ja = assmanj["objects"].toObject();
	assm = ja.size();
	for(QJsonObject::iterator it = ja.begin(); it != ja.end(); ++it){
		QJsonValue a = *it;
		QJsonObject jo = a.toObject();
		QString hash = jo["hash"].toString();
		QString chash = QString(hash.data()[0]) + QString(hash.data()[1]);
		QString apath = assPath + "objects/" + chash + "/" + hash;
		QUrl aurl = QUrl("http://resources.download.minecraft.net/" + chash + "/" + hash);
		++ass;
		if(!QFile::exists(getfilepath(apath))){
			downloadFile(aurl, apath);
			return;
		}
	}
	dp("Ready to play");
	changeProgressState(0, "Done.", false);
	disableControls(true);
	progstate = PState::INIT;
}

void MWin::libdown(){

	QJsonArray ja = currmanj["libraries"].toArray();
	for(QJsonArray::iterator it = ja.begin(); it != ja.end(); ++it){
		QJsonValue a = *it;
		QJsonObject jo = a.toObject();
		QString libPath = libsPath + jo["downloads"].toObject()["artifact"].toObject()["path"].toString();
		QUrl libUrl = QUrl(jo["downloads"].toObject()["artifact"].toObject()["url"].toString());
		currFile = libPath;
		
		if(!QFile::exists(getfilepath(libPath))){
			if(jo.contains("rules")){
#ifdef Q_OS_WIN
				if(jo["rules"].toArray()[0].toObject()["os"].toObject()["name"].toString() == "windows")
#elif defined(Q_OS_LINUX)
				if(jo["rules"].toArray()[0].toObject()["os"].toObject()["name"].toString() == "linux")
#else	
				if(jo["rules"].toArray()[0].toObject()["os"].toObject()["name"].toString() == "osx")
#endif
				{
					downloadFile(libUrl, libPath);
					return;
				}
			}else{
				downloadFile(libUrl, libPath);
				return;
			}
		}
	}
	changeProgressState(0, "Done.", false);
	progstate = PState::ASSDOWN;
	assdown();
}

void MWin::vermandown(){

	QFile f(getfilepath(currManFile));
	if (!f.open(QFile::ReadOnly | QFile::Text)) return;
	QTextStream in(&f);
	QString str = in.readAll();
	QJsonDocument jsonResponse = QJsonDocument::fromJson(str.toUtf8());
	currmanj = jsonResponse.object();
	QString dpath = verPath + vList->currentText() + "/" + vList->currentText() + ".jar";
	currFile = dpath;
	QString id = currmanj["assetIndex"].toObject()["id"].toString();
	QUrl uri = QUrl(currmanj["assetIndex"].toObject()["url"].toString());
	QString assPathFile = assPath + "/indexes/" + id + ".json";
	if(!QFile::exists(getfilepath(dpath)))
		downloadFile(QUrl(currmanj["downloads"].toObject()["client"].toObject()["url"].toString()), dpath);
	else{
		if(!QFile::exists(getfilepath(assPathFile)))
			downloadFile(uri, assPathFile);
		else{
			qDebug() << assPathFile;
			QFile f(getfilepath(assPathFile));
			if (!f.open(QFile::ReadOnly | QFile::Text)) return;
			QTextStream in(&f);
			QString str = in.readAll();
			QJsonDocument jsonResponse = QJsonDocument::fromJson(str.toUtf8());
			assmanj = jsonResponse.object();
			progstate = PState::LIBDOWN;
			libdown();
		}
	}

}

void MWin::progressFinish(){
	currFile = nullptr;
	switch(progstate){
		case PState::MANDOWN:
			manlistimport();
			break;
		case PState::MANCHEKSUM:
			changeProgressState(0, "Checking manifest checksum...", false);
			qDebug() << "Checking manifest checksum...";
			httpReq(QUrl("https://vilafox.xyz/api/yokaiLauncher?get=sha"));
			break;
		case PState::VERMANDOWN:
			vermandown();
			break;
		case PState::LIBDOWN:
			libdown();
			break;
		case PState::ASSDOWN:
			assdown();
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
	QDir d;
	QFileInfo fi(path);

	if(QFile::exists(path))
		QFile::remove(path);

	if(!d.mkpath(fi.dir().path()))
		return;

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
	dp("Play button clicked");
	disableControls();
	QString dpath = verPath + vList->currentText();
	QString manifestUri = vData.value(vList->currentText());
	QDir d;
	if(d.mkpath(getfilepath(dpath))){
		progstate = PState::VERMANDOWN;
		currManFile = dpath + "/" + vList->currentText() + ".json";
		if(!QFile::exists(getfilepath(dpath + "/" + vList->currentText() + ".json")))
			downloadFile(QUrl(manifestUri), dpath + "/" + vList->currentText() + ".json");
		else
			vermandown();
	}
	
}

void MWin::appshow(){
	ui_mw->show();
	dp("UI loaded");
#ifdef Q_OS_WIN
	dp("Win show taskbar icon");
	QWinTaskbarButton *button = new QWinTaskbarButton(this);
    button->setWindow(windowHandle());
    button->setOverlayIcon(QIcon(":/assets/icon.svg"));
#endif
	disableControls();
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
