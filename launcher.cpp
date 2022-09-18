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
	QList<QString> flist;
	flist.append(":/assets/e-Ukraine-Bold.otf");
	flist.append(":/assets/e-Ukraine-Thin.otf");
	flist.append(":/assets/e-Ukraine-Light.otf");
	flist.append(":/assets/e-UkraineHead-LOGO.otf");
	for(QList<QString>::iterator it = flist.begin(); it != flist.end(); ++it){
		QFile file(*it);
		file.open(QFile::ReadOnly);
		QFontDatabase::addApplicationFontFromData(file.readAll());
	}
	return 1;
}

void MWin::verChanged(const QString &text){
	config->setVal("lastver", text);
}

void MWin::isFabricbox(int state){
	if(state == Qt::Checked)
		config->setVal("isFabric", true);
	else
		config->setVal("isFabric", false);
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
	fabricb = findChild<QCheckBox*>("fabricb");
	settingsb = findChild<QPushButton*>("settb");
	bid = findChild<QLabel*>("bid");
	bid->setText("Build #" + QString::number(BUILDID));
	this->setWindowFlags(Qt::Window);
	QMetaObject::connectSlotsByName( this );
	connect(vList, &QComboBox::currentTextChanged, this, &MWin::verChanged);
	connect(fabricb, &QCheckBox::stateChanged, this, &MWin::isFabricbox);
}

MWin::~MWin(){
	if(run) process->kill();
	if(!mm->isMaximized()){
		config->setVal("height", mm->height());
		config->setVal("width", mm->width());
	}
	config->setVal("maximized", mm->isMaximized());
	dp("===============================\n");
}


void MWin::disableControls(bool a = true){
	bool val = !a;
	nickname->setEnabled(val);
	vList->setEnabled(val);
	playBtn->setEnabled(val);
	fabricb->setEnabled(val);
	settingsb->setEnabled(val);
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

QStringList MWin::getJargs(){
	QString args = config->getVal("jvmargs");
	args.append("-Dminecraft.launcher.brand=yokai");
	args.append(" -Dminecraft.launcher.version=alpha-0");
	QString assetsver = currmanj["assets"].toString();
	QString ver = currmanj["id"].toString();
	bool isFabric = QVariant(config->getVal("isFabric")).toBool();
	if(isFabric) args.append(" -DFabricMcEmu="+currmanj["mainClass"].toString());
	QString mainclass = isFabric ? fabMclass : currmanj["mainClass"].toString();
	QString libs = "-cp ";
	QJsonArray ja = currmanj["libraries"].toArray();
	for(QJsonArray::iterator it = ja.begin(); it != ja.end(); ++it){
		QJsonValue a = *it;
		QJsonObject jo = a.toObject();
		QString libPath = getfilepath(Path.libsPath + jo["downloads"].toObject()["artifact"].toObject()["path"].toString());
		if(jo["downloads"].toObject().contains("classifiers")){
#ifdef Q_OS_WIN
			QString p = "windows";
#else
			QString p = "linux";
#endif
			if(jo["downloads"].toObject()["classifiers"].toObject().contains("natives-"+p)){
				libs.append(libPath + ":");
				QString u = jo["downloads"].toObject()["classifiers"].toObject()["natives-"+p].toObject()["url"].toString();
				QString pp = Path.libsPath + jo["downloads"].toObject()["classifiers"].toObject()["natives-"+p].toObject()["path"].toString();
				libs.append(getfilepath(pp) + ":");
			}
		}
		if(jo.contains("rules")){
#ifdef Q_OS_WIN
			if(jo["rules"].toArray()[0].toObject()["os"].toObject()["name"].toString() == "windows")
#elif defined(Q_OS_LINUX)
			if(jo["rules"].toArray()[0].toObject()["os"].toObject()["name"].toString() == "linux")
#else	
			if(jo["rules"].toArray()[0].toObject()["os"].toObject()["name"].toString() == "osx")
#endif
			{
				libs.append(libPath + ":");
			}
		}else
			libs.append(libPath + ":");
	}
	
	if(isFabric) libs.append(fabLibs);
	args.append(" " + libs + getfilepath(Path.verPath+ver+"/"+ver+".jar"));
	args.append(" " + mainclass);

	args.append(" --username " + nickname->text());
	args.append(" --version " + ver);
	args.append(" --gameDir " + Path.mcPath);
	args.append(" --assetsDir " + getfilepath(Path.assPath));
	args.append(" --assetIndex " + assetsver);
	args.append(" --uuid " + QUuid::createUuidV3(QUuid(), nickname->text()).toString().replace("{", "").replace("}", ""));
	args.append(" --accessToken null"); //TODO: mojang auth
	args.append(" --userType legacy");
	args.append(" --versionType " + currmanj["type"].toString());

	return args.split(" ");
}

void MWin::fabricDownload(){
	QString fabricMaven = "https://maven.fabricmc.net/";
	QString fabricMcMavenDir = "net/fabricmc/";
	QString fabricMavenDir = fabricMcMavenDir + "fabric-loader/";
	QString fp = "fabric/fabric-loader-"+fabVer;
	QString path = fp +".json";
	QString fab = fabricMaven+fabricMavenDir+fabVer+"/fabric-loader-"+fabVer;
	progstate = PState::FabricDown;
	if(!QFile::exists(getfilepath(path))){
		QFileInfo fi(getfilepath(path));
		QDir dir(fi.dir().path());
		dir.setNameFilters(QStringList() << "fabric-loader-*.*");
		dir.setFilter(QDir::Files);
		foreach(QString dirFile, dir.entryList())
		{
			dir.remove(dirFile);
		}
		downloadFile(fab+".json", path);
		return;
	}

	if(!QFile::exists(getfilepath(fp+".jar"))){
		currFile = getfilepath(fp+".jar");
		downloadFile(fab+".jar", fp+".jar");
		return;
	}

	QFile f(getfilepath(path));
	if (!f.open(QFile::ReadOnly | QFile::Text)) return;
	QTextStream in(&f);
	QString str = in.readAll();
	QJsonDocument jsonResponse = QJsonDocument::fromJson(str.toUtf8());
	QJsonArray ja = jsonResponse.object()["libraries"].toObject()["common"].toArray();
	fabMclass = jsonResponse.object()["mainClass"].toObject()["client"].toString();
	for(QJsonArray::iterator it = ja.begin(); it != ja.end(); ++it){
		QJsonValue a = *it;
		QJsonObject jo = a.toObject();
		QStringList libMaven = jo["name"].toString().split(":");
		QString apath = libMaven[0].replace(".", "/");
		QString name = libMaven[1];
		QString ver = libMaven[2];
		QString qpath = apath + "/" + name + "/" + ver + "/" + name + "-" + ver + ".jar";
		QString libPath = Path.libsPath + qpath;
		fabLibs.append(getfilepath(libPath)+":");
		QUrl libUrl = QUrl(fabricMaven + qpath);
		currFile = libPath;
		if(!QFile::exists(getfilepath(libPath))){
			downloadFile(libUrl, libPath);
			return;
		}
	}
	

	QString qpath = fabricMcMavenDir + "/intermediary/" + currmanj["id"].toString() + "/intermediary-" + currmanj["id"].toString() + "-v2.jar";
	QString libPath = Path.libsPath + qpath;

	if(!QFile::exists(getfilepath(libPath))){
		currFile = getfilepath(libPath);
		downloadFile(fabricMaven + qpath, libPath);
		return;
	}

	dp("Fabric enabled");
	fabLibs.append(getfilepath(libPath)+":");
	fabLibs.append(getfilepath(fp + ".jar")+":");
	launch();
}

bool MWin::checkJava(){
	return (QString::fromLocal8Bit(qgetenv("JAVA_HOME")) == "") ? false : true;
}

void MWin::mcend(int exitCode, QProcess::ExitStatus ExitStatus){
	disableControls(false);
	changeProgressState(0, "Game exit", false);
	qDebug() << "Exit code: " << exitCode;
	qDebug() << "Exit status: " << ExitStatus;
	run = false;
	bool ism = QVariant(config->getVal("maximized")).toBool();
	if(ism)
		ui_mw->setWindowState(Qt::WindowMaximized);
	else
		ui_mw->setWindowState(ui_mw->windowState() & ~Qt::WindowMinimized);
}

void MWin::launch(){
	if(!checkJava()){
		qFatal("Err: Java not found!");
		return;
	}
	QString java_home = QString::fromLocal8Bit(qgetenv("JAVA_HOME"));
	#ifdef Q_OS_WIN
	QString jvm = java_home + "/bin/javaw";
	#else
	QString jvm = java_home + "/bin/java";
	#endif
	QStringList args = getJargs();
	process = new QProcess(this);
	run = true;
	connect(process, SIGNAL(finished(int , QProcess::ExitStatus )), this, SLOT(mcend(int , QProcess::ExitStatus )));
	ui_mw->setWindowState(Qt::WindowMinimized);
	changeProgressState(0, "Launching...", false);
	disableControls();
	process->start(QDir::cleanPath(jvm), args);
	// process->execute(QDir::cleanPath(jvm), args);
	
}

QString MWin::getfilepath(QString path){
	QString ddir = QDir::cleanPath(Path.mcPath);
	QDir d;
	if(d.mkpath(ddir))
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
	progressFinish();
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
		changeProgressState(static_cast<int>(ass), static_cast<int>(assm), "Downloading assets " + QString::number(static_cast<int>(ass)) + "/" + QString::number(static_cast<int>(assm)));
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
	qDebug() << "Checksum local: " << a;
	qDebug() << "Checksum server: " << mansha;
	if(a != mansha){
		f.remove();
		purl();
		return;
	}
	QJsonDocument jsonResponse = QJsonDocument::fromJson(str.toUtf8());
	QJsonObject jsonObject = jsonResponse.object();
	QString latestVersion = jsonObject["latest"].toObject()["release"].toString();
	fabVer = jsonObject["fabric"].toString();
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
	vList->setCurrentText(config->getVal("lastver"));
	changeProgressState(false);
	disableControls(false);
}

bool MWin::isWhiteSpace(const QString & str)
{
	return (QRegExp("\\s.").indexIn(str) != -1) ? true : false;
}


void MWin::assdown(){
	QJsonObject ja = assmanj["objects"].toObject();
	assm = ja.size();
	ass = 0;
	for(QJsonObject::iterator it = ja.begin(); it != ja.end(); ++it){
		++ass;
		QJsonValue a = *it;
		QJsonObject jo = a.toObject();
		QString hash = jo["hash"].toString();
		QString chash = QString(hash.data()[0]) + QString(hash.data()[1]);
		QString apath = Path.assPath + "objects/" + chash + "/" + hash;
		QUrl aurl = QUrl("http://resources.download.minecraft.net/" + chash + "/" + hash);
		if(!QFile::exists(getfilepath(apath))){
			downloadFile(aurl, apath);
			return;
		}
	}
	ass = 0;
	assm = 0;
	dp("Ready to play");
	disableControls(false);
	progstate = PState::INIT;
	if(isWhiteSpace(nickname->text())){
		dp("Warn: spaces in nickname");
		changeProgressState(0, "Warn: spaces in nickname", false);
		return;
	}
	bool isFabric = QVariant(config->getVal("isFabric")).toBool();
	if(isFabric){
		fabricDownload();
		return;
	}
	else launch();
}

void MWin::libdown(){

	QJsonArray ja = currmanj["libraries"].toArray();
	for(QJsonArray::iterator it = ja.begin(); it != ja.end(); ++it){
		QJsonValue a = *it;
		QJsonObject jo = a.toObject();
		QString libPath = Path.libsPath + jo["downloads"].toObject()["artifact"].toObject()["path"].toString();
		QUrl libUrl = QUrl(jo["downloads"].toObject()["artifact"].toObject()["url"].toString());
		currFile = libPath;
		
		if(jo["downloads"].toObject().contains("classifiers")){
#ifdef Q_OS_WIN
			QString p = "windows";
#else
			QString p = "linux";
#endif
			if(jo["downloads"].toObject()["classifiers"].toObject().contains("natives-"+p)){
				if(!QFile::exists(getfilepath(libPath))){
					downloadFile(libUrl, libPath);
					return;
				}
				QString u = jo["downloads"].toObject()["classifiers"].toObject()["natives-"+p].toObject()["url"].toString();
				QString pp = Path.libsPath + jo["downloads"].toObject()["classifiers"].toObject()["natives-"+p].toObject()["path"].toString();
				if(!QFile::exists(getfilepath(pp))){
					downloadFile(u, pp);
					return;
				}
			}
		}
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
	QString dpath = Path.verPath + vList->currentText() + "/" + vList->currentText() + ".jar";
	currFile = dpath;
	QString id = currmanj["assetIndex"].toObject()["id"].toString();
	QUrl uri = QUrl(currmanj["assetIndex"].toObject()["url"].toString());
	QString assPathFile = Path.assPath + "/indexes/" + id + ".json";
	if(!QFile::exists(getfilepath(dpath)))
		downloadFile(QUrl(currmanj["downloads"].toObject()["client"].toObject()["url"].toString()), dpath);
	else{
		if(!QFile::exists(getfilepath(assPathFile)))
			downloadFile(uri, assPathFile);
		else{
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
			purl();
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
		case PState::READY2PLAY:
			launch();
			break;
		case PState::FabricDown:
			fabricDownload();
			break;
		default:
			changeProgressState(0, "Done.", false);
			disableControls(false);
	}
}

void MWin::httpReq(const QUrl &requestedUrl) {
	reply = qnam.get(QNetworkRequest(requestedUrl));
	if(progstate != PState::MANCHEKSUM){
		connect(reply, &QIODevice::readyRead, this, &MWin::httpRead);
		connect(reply, &QNetworkReply::downloadProgress, this, &MWin::progress_func);
		connect(reply, &QNetworkReply::finished, this, &MWin::httpFinish);
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
	if(!QFile::exists(getfilepath(manpath))){
		progstate = PState::MANDOWN;
		currFile = manpath;
		dp("Downloading manifest...");
		downloadFile(QUrl("https://vilafox.xyz/api/yokaiLauncher"), manpath);
	}else{	
		progstate = PState::MANCHEKSUM;
		progressFinish();
	}

}

void MWin::on_playBtn_clicked(){
	disableControls();
	config->setVal("nickname", nickname->text());
	QString dpath = Path.verPath + vList->currentText();
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

void MWin::loadconf()
{
	QString cpath = getfilepath("yokai.yml");
	QDir d;
	config = new CMan();
	config->load(cpath);
	Path.mcPath = config->getVal("mcdir");
	bool ism = QVariant(config->getVal("maximized")).toBool();
	bool ifc = QVariant(config->getVal("isFabric")).toBool();
	if(ism){
		ui_mw->showMaximized();
	}else{
		ui_mw->show();
		ui_mw->resize(config->getVal("width").toInt(), config->getVal("height").toInt());
	}
	nickname->setText(config->getVal("nickname"));
	Qt::CheckState cs = ifc ? Qt::Checked : Qt::Unchecked;
	fabricb->setCheckState(cs);
	appshow();
}

int main(int argc, char *argv[])
{
	dp("\n======== yokaiLauncher ========");
	dp("Init");
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication app(argc, argv);
	app.setStyleSheet(importQss());
	app.setDesktopFileName("xyz.vilafox.mc.yokaiLauncher");
	app.setWindowIcon(QIcon(":/assets/icon.svg"));
	importFonts();
	MWin n;
	app.setActiveWindow(&n);
	n.progstate = PState::INIT;
	n.loadconf();
	return app.exec();
}
