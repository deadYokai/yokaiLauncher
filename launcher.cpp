#include <launcher.h>
#include <fstream>
#include <QDialog>
#include <limits>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

// Custom debug outputs (idk why i'm need this)
void dp(std::string a){	std::cout << a << std::endl;}
void dp(char a){std::cout << a << std::endl;}
void dp(int a){std::cout << a << std::endl;}
void dp(double a){std::cout << a << std::endl;}

static QWidget *loadUiFile(QString page, QWidget *parent = nullptr)
{
	QFile file(":/assets/" + page + ".ui");
	file.open(QIODevice::ReadOnly);

	QUiLoader loader;
	if(parent == nullptr) return loader.load(&file);
	else return loader.load(&file, parent);
}

QString MWin::importStyle(QString stylePath){
	QFile file(stylePath);
	file.open(QFile::ReadOnly | QFile::Text);
	QTextStream in(&file);
	QFileInfo fi(file.fileName());
	QString styleSheet = in.readAll().replace("path:", fi.path() + "/");
	file.close();
	QRegularExpression *re = new QRegularExpression("background-covered:(.*?);");
	QRegularExpressionMatch r = re->match(styleSheet);
	if(r.hasMatch()){
		QRegularExpressionMatch bpathm = QRegularExpression("\"(.*?)\"").match(r.captured(1));
		if(bpathm.hasMatch()){
			QString bpath = bpathm.captured(1);
			if(QFile::exists(bpath))
				_pixmapBg.load(bpath);
			else{
				qDebug() << "Warn: Background image not found on "+bpath+", setting to default";
				_pixmapBg.load(":/assets/bg_1.png");
			}
		}else{
			qDebug() << "Warn: Invalid background image, setting to default";
			_pixmapBg.load(":/assets/bg_1.png");
		}
	}
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

// Find RAM
uint64_t getSystemRam()
{
#ifdef Q_OS_WIN
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx( &status );
	return (uint64_t)status.ullTotalPhys;
#else
	std::string token;
	std::ifstream file("/proc/meminfo");
	while(file >> token)
	{
		if(token == "MemTotal:")
		{
			uint64_t mem;
			if(file >> mem)
			{
				return mem * 1024ull;
			}
			else
			{
				return 0;
			}
		}
		file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	return 0;
#endif
}


// Settings Button event
void MWin::settbtn_click(){

	QCheckBox *isf = settingsWidget->findChild<QCheckBox*>("isMcFullscreen");
	QLineEdit *jvma = settingsWidget->findChild<QLineEdit*>("jvmaddargs");
	QSlider *rams = settingsWidget->findChild<QSlider*>("ramslider");
	QLabel *mml = settingsWidget->findChild<QLabel*>("maxmemLabel");
	QLabel *cram = settingsWidget->findChild<QLabel*>("currRam");

	bool ifsc = QVariant(config->getVal("isFullscreen")).toBool();
	isf->setCheckState(ifsc ? Qt::Checked : Qt::Unchecked);

	jvma->setText(config->getVal("jvmargs"));

	uint64_t mbRam = getSystemRam() / (1024ul * 1024ul);	

	rams->setMaximum(mbRam);
	rams->setMinimum(2048);
	connect(rams, &QSlider::valueChanged, [=](int value){
		cram->setText("Current RAM: " + QString::number(value) + "MB");
	});

	mml->setText(QString::number(mbRam/1024) + "GB");
	cram->setText("Current RAM: " + QString::number(rams->value()) + "MB");

	pWidget->hide();
	mwCW->hide();
	bwi->hide();
	uwi->show();
}

void MWin::disableControls(bool a = true){
	bool val = !a;
	nickname->setEnabled(val);
	vList->setEnabled(val);
	playBtn->setEnabled(val);
	fabricb->setEnabled(val);
	settingsb->setEnabled(val);
}


// UI & Events Init
MWin::MWin(QWidget *parent) : QMainWindow(parent)
{

	this->setWindowFlags(Qt::Window);
	this->setWindowModality(Qt::ApplicationModal);

	ui_mw = loadUiFile("client", this);
	mm = findChild<QWidget*>("ClientForm");
	mwCW = findChild<QWidget*>("Logo");
	nickname = findChild<QLineEdit*>("nickname");
	pWidget = findChild<QWidget*>("progressWidget");
	vList = findChild<QComboBox*>("versionList");
	pLabel = findChild<QLabel*>("pLabel");
	bwi = findChild<QWidget*>("bottomWidget");
	progressBar = findChild<QProgressBar*>("progressBar");
	playBtn = findChild<QPushButton*>("playBtn");
	fabricb = findChild<QCheckBox*>("fabricb");
	settingsb = findChild<QPushButton*>("settb");
	uwi = findChild<QWidget*>("widget");
	
	QVBoxLayout *l = new QVBoxLayout(uwi);
	uwi->setLayout(l);
	settingsWidget = loadUiFile("settw", uwi);
	l->addWidget(settingsWidget);
	uwi->close();
	
	themeBox = settingsWidget->findChild<QComboBox*>("themesBox");
	mcPathEdit = settingsWidget->findChild<QLineEdit*>("mcPathEdit");
	mcPathSel = settingsWidget->findChild<QPushButton*>("mcPathSelect");
	mcFolBtn = settingsWidget->findChild<QPushButton*>("mcFolderBtn");

	bid = new QLabel(ui_mw);
	bid->setStyleSheet("font-size: 14px");
	bid->setGeometry(8, 8, 1000, 32);
	settsavebtn = settingsWidget->findChild<QPushButton*>("settsaveb");
	bid->setText("Build #" + QString::number(BUILDID));

	ui_mw->setWindowFlags(Qt::Widget);
	
	connect(vList, &QComboBox::currentTextChanged, this, &MWin::verChanged);
	connect(fabricb, &QCheckBox::stateChanged, this, &MWin::isFabricbox);
	connect(settingsb, &QPushButton::clicked, this, &MWin::settbtn_click);

	connect(mcFolBtn, &QPushButton::clicked, this, [=](){ QDesktopServices::openUrl(QUrl("file:///"+config->getVal("mcdir"), QUrl::TolerantMode)); });
	connect(playBtn, &QPushButton::clicked, this, [=](){
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
	 });
	connect(mcPathSel, &QPushButton::clicked, this, [=](){ 
		QFileDialog *dir = new QFileDialog(this);
		dir->setFileMode(QFileDialog::Directory);
		dir->setOption(QFileDialog::ShowDirsOnly);
		dir->setViewMode(QFileDialog::List);
		dir->setDirectory(Path.mcPath);
		if(dir->exec()){
			if(!dir->selectedFiles()[0].isEmpty()){
				mcPathEdit->setText(dir->selectedFiles()[0]);
			}
		}
	 });
	connect(settsavebtn, &QPushButton::clicked, this, [=](){
		QCheckBox *isf = settingsWidget->findChild<QCheckBox*>("isMcFullscreen");
		QLineEdit *jvma = settingsWidget->findChild<QLineEdit*>("jvmaddargs");
		QSlider *rams = settingsWidget->findChild<QSlider*>("ramslider");
		if(themeBox->currentIndex() > 0){
			qDebug() << themeBox->currentIndex();
			config->setVal("theme", themeBox->currentText());
			QDir *tpath = new QDir(getfilepath("themes"));
			QString themepath = tpath->path() + "/" + config->getVal("theme")+"/"+config->getVal("theme")+".theme.css";
			setStyleSheet(importStyle(themepath));
		}else{
			config->setVal("theme", "");
			setStyleSheet("");
		}
		QDir q;
		if(q.mkpath(mcPathEdit->text())){
			config->setVal("mcdir", mcPathEdit->text());
			Path.mcPath = mcPathEdit->text();
		}else{
			msgBox("Wrong Minecraft Path. Not Saved");
		}
		config->setVal("ram", rams->value());
		config->setVal("isFullscreen", ((isf->checkState() == Qt::Checked) ? 1 : 0));
		config->setVal("jvmargs", jvma->text());
		pWidget->show();
		mwCW->show();
		bwi->show();
		uwi->close();
	});

}


void MWin::paintEvent(QPaintEvent *pe)
{
	QPainter painter(this);

	auto winSize = ui_mw->size();
	auto pixmapRatio = (float)_pixmapBg.width() / _pixmapBg.height();
	auto windowRatio = (float)winSize.width() / winSize.height();

	if(pixmapRatio > windowRatio)
	{
		auto newWidth = (int)(winSize.height() * pixmapRatio);
		auto offset = (newWidth - winSize.width()) / -2;
		painter.drawPixmap(offset, 0, newWidth, winSize.height(), _pixmapBg);
	}
	else
	{
		auto newHeight = (int)(winSize.width() / pixmapRatio);
		painter.drawPixmap(0, 0, winSize.width(), newHeight, _pixmapBg);
	}


}


void MWin::closeEvent (QCloseEvent *event)
{
    qDebug() << "Bye...";
}

MWin::~MWin(){
	if(run) process->kill();
	if(!isMaximized()){
		config->setVal("height", height());
		config->setVal("width", width());
	}
	config->setVal("maximized", isMaximized());
	dp("===============================\n");
}

// Custom MessageBox
void MWin::msgBox(QString msg){
	QDialog *box = new QDialog(ui_mw);
	box->setWindowModality(Qt::WindowModal);
	QPushButton *btn = new QPushButton("OK", box);
	QLabel *label = new QLabel(msg, box);
	QVBoxLayout *lay = new QVBoxLayout();
	lay->addWidget(label);
	lay->addWidget(btn);
	box->setLayout(lay);
	btn->setStyleSheet("font-size: 18px");
	label->setStyleSheet("font-size: 16px");
	btn->setMaximumHeight(32);
	lay->setAlignment(label, Qt::AlignHCenter | Qt::AlignVCenter);
	box->setFixedSize(500, 300);
	box->open();
	connect(btn, &QPushButton::clicked, this, [=]() { 
		box->close();
	});
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

bool MWin::checksha1(QString path, QString sha1){
	QFile f(path);
	if(!f.open(QFile::ReadOnly)) return false;
	QByteArray str = f.readAll();
	QCryptographicHash hash(QCryptographicHash::Sha1);
	hash.addData(str);
	QString a = hash.result().toHex();
	return (sha1 == a);
}

// Generate Java arguments
QStringList MWin::getJargs(){
	QString args = config->getVal("jvmargs");
	args.append("-Dminecraft.launcher.brand=yokai");
	args.append(" -Dminecraft.launcher.version=alpha-0");
	args.append(" -Xms2G");
	QString mem = config->getVal("ram");
	args.append(" -Xmx" + mem + "M");
	QString assetsver = currmanj["assets"].toString();
	QString ver = currmanj["id"].toString();
	QString paths = ":";
	#ifdef Q_OS_WIN
	paths = ";";
	#endif
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
				libs.append(libPath + paths);
				QString pp = Path.libsPath + jo["downloads"].toObject()["classifiers"].toObject()["natives-"+p].toObject()["path"].toString();
				libs.append(getfilepath(pp) + paths);
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
				libs.append(libPath + paths);
			}
		}else
			libs.append(libPath + paths);
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

	bool isF = QVariant(config->getVal("isFullscreen")).toBool();
	if(isF) args.append(" --fullscreen");
	
	// qDebug() << args;
	return args.split(" ");
}

// Check Fabric and Download
void MWin::fabricDownload(){
	QString fabricMaven = "https://maven.fabricmc.net/";
	QString fabricMcMavenDir = "net/fabricmc/";
	QString fabricMavenDir = fabricMcMavenDir + "fabric-loader/";
	QString fp = ".fabric/fabric-loader-"+fabVer;
	QString path = fp +".json";
	QString fab = fabricMaven+fabricMavenDir+fabVer+"/fabric-loader-"+fabVer;
	progstate = PState::FabricDown;
	if(!QFile::exists(getfilepath(path))){
		QFileInfo fi(getfilepath(path));
		QDir dir(fi.dir().path());
		dir.setNameFilters(QStringList() << "fabric-loader-*.*\\.jar" << "fabric-loader-*.*\\.json");
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
	QString paths = ":";
	#ifdef Q_OS_WIN
	paths = ";";
	#endif
	for(QJsonArray::iterator it = ja.begin(); it != ja.end(); ++it){
		QJsonValue a = *it;
		QJsonObject jo = a.toObject();
		QStringList libMaven = jo["name"].toString().split(":");
		QString apath = libMaven[0].replace(".", "/");
		QString name = libMaven[1];
		QString ver = libMaven[2];
		QString qpath = apath + "/" + name + "/" + ver + "/" + name + "-" + ver + ".jar";
		QString libPath = Path.libsPath + qpath;
		fabLibs.append(getfilepath(libPath)+paths);
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
	fabLibs.append(getfilepath(libPath)+paths);
	fabLibs.append(getfilepath(fp + ".jar")+paths);
	checkJava();
}

// Check Java and Downloads
void MWin::checkJava(){
	javaRuntimeDir = QString::fromLocal8Bit(qgetenv("JAVA_HOME"));
	qDebug() << "Java init";
#ifdef Q_OS_WIN
	
	QUrl jmanifest = QUrl("https://launchermeta.mojang.com/v1/products/java-runtime/2ec0cc96c44e5a76b9c8b7c39df7210883d12871/all.json");
	QString winp = "x86";
	#ifdef Q_OS_WIN64
		winp = "x64";
	#endif
	progstate = PState::JAVAIN;

	if(!QFile::exists(getfilepath("java/all.json"))){
		currFile = getfilepath("java/all.json");
		downloadFile(jmanifest, "java/all.json");
		return;
	}
	
	
	QString runtimever = currmanj["javaVersion"].toObject()["component"].toString();
	if(!QFile::exists(getfilepath("java/"+runtimever+"/windows-"+winp+".json"))){
		currFile = getfilepath("java/"+runtimever+"/windows-"+winp+".json");
		QFile f(getfilepath("java/all.json"));
		if (!f.open(QFile::ReadOnly | QFile::Text)) return;
		QTextStream in(&f);
		QString str = in.readAll();
		QJsonDocument jsonResponse = QJsonDocument::fromJson(str.toUtf8());
		QUrl runtimeurl = QUrl(jsonResponse.object()["windows-"+winp].toObject()[runtimever].toArray()[0].toObject()["manifest"].toObject()["url"].toString());
		downloadFile(runtimeurl, "java/"+runtimever+"/windows-"+winp+".json");
		f.close();
		return;
	}

	QFile f(getfilepath("java/"+runtimever+"/windows-"+winp+".json"));
	if (!f.open(QFile::ReadOnly | QFile::Text)) return;
	QTextStream in(&f);
	QString str = in.readAll();
	QJsonDocument jsonResponse = QJsonDocument::fromJson(str.toUtf8());
	QJsonObject files = jsonResponse.object()["files"].toObject();
	QStringList keys = files.keys();
	for (QStringList::iterator it = keys.begin(); it != keys.end(); ++it) {
		QString b = *it;
		QJsonObject a = files[b].toObject();
		if(a["type"] == "file"){
			if(!QFile::exists(getfilepath("java/"+runtimever+"/windows-"+winp+"/"+b))){
				QUrl uri = QUrl(a["downloads"].toObject()["raw"].toObject()["url"].toString());
				currFile = getfilepath("java/"+runtimever+"/windows-"+winp+"/"+b);
				downloadFile(uri, "java/"+runtimever+"/windows-"+winp+"/"+b);
				return;
			}
		}
	}	

	javaRuntimeDir = getfilepath("java/"+runtimever+"/windows-"+winp+"/");

	progstate = PState::INIT;

#endif
	launch();
}

// After Minecraft close
void MWin::mcend(int exitCode, QProcess::ExitStatus ExitStatus){
	disableControls(false);
	changeProgressState(0, "Game exit", false);
	if(debug){
		QPlainTextEdit *pp = dcon->findChild<QPlainTextEdit*>("debugT");
		pp->appendPlainText("Exit code: " + QString::number(exitCode));
	}
	qDebug() << "Exit code: " << exitCode;
	qDebug() << "Exit status: " << ExitStatus;
	run = false;
	bool ism = QVariant(config->getVal("maximized")).toBool();
	if(ism)
		setWindowState(Qt::WindowMaximized);
	else
		setWindowState(windowState() & ~Qt::WindowMinimized);
}

// Write Minecraft output error to DebugLog
void MWin::re(){
	QProcess *p = qobject_cast<QProcess*>(sender());
	p->setReadChannel(QProcess::StandardError);
	QPlainTextEdit *pp = dcon->findChild<QPlainTextEdit*>("debugT");
	while(p->canReadLine())
	{
		pp->appendPlainText(p->readLine().replace("\n", ""));
	}

}

// Write Minecraft output to DebugLog
void MWin::rr(){
	QProcess *p = qobject_cast<QProcess*>(sender());
	p->setReadChannel(QProcess::StandardOutput);
	QPlainTextEdit *pp = dcon->findChild<QPlainTextEdit*>("debugT");
	pp->appendPlainText("Minecraft launching...");
	while(p->canReadLine())
	{
		pp->appendPlainText(p->readLine().replace("\n", ""));
	}

}

// Launch Minecraft
void MWin::launch(){

	QString jvm = javaRuntimeDir + "/bin/java";
	qDebug() << "Launching...";
	
	#ifdef Q_OS_WIN
	if(!debug) jvm.append("w");
	jvm.append(".exe");
	#endif
	QStringList args = getJargs();
	process = new QProcess(this);
	run = true;
	if(debug){
		QPlainTextEdit *pp = dcon->findChild<QPlainTextEdit*>("debugT");
		pp->appendPlainText("Launching...");
	}
	connect(process, SIGNAL(finished(int , QProcess::ExitStatus )), this, SLOT(mcend(int , QProcess::ExitStatus )));
	if(debug){
		connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(rr()));
		connect(process, SIGNAL(readyReadStandardError()), this, SLOT(re()));
	}else
		setWindowState(Qt::WindowMinimized);
	changeProgressState(0, "Launching...", false);
	disableControls();
	process->start(QDir::cleanPath(jvm), args);
	
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

// After http request
void MWin::httpFinish(){
	QFileInfo fi;
	if(progstate != PState::MANCHEKSUM){

		if (file) {
			fi.setFile(file->fileName());
			file->close();
			file.reset();
		}
		if (reply->error()) {
			if(reply->error() == QNetworkReply::ContentNotFoundError && progstate == PState::FabricDown){
				msgBox("Fabric not supported in version: "+currmanj["id"].toString());
				progstate = PState::INIT;
				progressFinish();
			}
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
			QString s = "Downloading: ";
			if(progstate == PState::JAVAIN){
				s = "Installing java: ";
			}
			changeProgressState((int)bytesRead, (int)totalBytes, s + fi.fileName());
		}
		else
			changeProgressState((int)bytesRead, (int)totalBytes, QString::number((int)bytesRead)+"/"+QString::number((int)totalBytes));
	}
}

// Load main manifest
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
	return QRegularExpression("\\s.").match(str).hasMatch();
}

// Download assets for Minecrafts
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

		if(!checksha1(getfilepath(apath), hash)){
			downloadFile(aurl, apath);
			return;
		}
	}
	ass = 0;
	assm = 0;
	dp("Ready to play");
	progstate = PState::INIT;
	if(isWhiteSpace(nickname->text())){
		dp("Warn: spaces in nickname");
		changeProgressState(0, "Warn: spaces in nickname", false);
		msgBox("Don't use spaces in nicknames");
		disableControls(false);
		return;
	}
	if(nickname->text().isEmpty()){
		msgBox("Enter nickname");
		disableControls(false);
		return;
	}
	bool isFabric = QVariant(config->getVal("isFabric")).toBool();
	if(isFabric){
		fabricDownload();
		return;
	}
	else checkJava();
}

// Download Java libraries
void MWin::libdown(){

	QJsonArray ja = currmanj["libraries"].toArray();
	for(QJsonArray::iterator it = ja.begin(); it != ja.end(); ++it){
		QJsonValue a = *it;
		QJsonObject jo = a.toObject();
		QString libPath = Path.libsPath + jo["downloads"].toObject()["artifact"].toObject()["path"].toString();
		QString libSha1 = jo["downloads"].toObject()["artifact"].toObject()["sha1"].toString();
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

		if(!checksha1(getfilepath(libPath), libSha1)){
			downloadFile(libUrl, libPath);
			return;
		}
	}
	progstate = PState::ASSDOWN;
	assdown();
}

// Check minecraft version and download
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

// What i'm do after http request
void MWin::progressFinish(){
	currFile = nullptr;
	switch(progstate){
		case PState::MANDOWN:
			purl();
			break;
		case PState::MANCHEKSUM:
			if(offline) manlistimport();
			else{
				changeProgressState(0, "Checking manifest checksum...", false);
				qDebug() << "Checking manifest checksum...";
				httpReq(QUrl("https://vilafox.xyz/api/yokaiLauncher?get=sha"));
			}
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
			checkJava();
			break;
		case PState::JAVAIN:
			checkJava();
			break;
		case PState::FabricDown:
			fabricDownload();
			break;
		default:
			changeProgressState(0, "Done.", false);
			disableControls(false);
	}
}

// Connect http request events
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

// Download init
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

// Check manifest file
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


// UI Checks
void MWin::appshow(){
	dp("UI loaded");

	if(debug){
		dcon = loadUiFile("dcon", ui_mw);
		dcon->setWindowModality(Qt::NonModal);
		dcon->setWindowFlags(Qt::Window);
		dcon->show();
	}

	disableControls();
	purl();
	
}

// Config loader & Init app
void MWin::loadconf()
{
	dp("\n======== yokaiLauncher ========");
	dp("Init");
	QString cpath = getfilepath("yokai.yml");
	QDir d;
	config = new CMan();
	config->load(cpath);
	Path.mcPath = config->getVal("mcdir");
	mcPathEdit->setText(Path.mcPath);
	bool ism = QVariant(config->getVal("maximized")).toBool();
	bool ifc = QVariant(config->getVal("isFabric")).toBool();
	if(!debug){
		try{
			debug = QVariant(config->getVal("_debug")).toBool();
		}catch(const std::exception& e){
			debug = false;
		}
	}
	if(debug){
		dp("=== DEBUG MODE ENABLED ===");
		bid->setText(bid->text() + " | DEBUG MODE");
	}

	if(ism){
		showMaximized();
	}else{
		show();
		resize(config->getVal("width").toInt(), config->getVal("height").toInt());
	}
	
	d.mkpath(getfilepath("themes/"));
	
	QDir *tpath = new QDir(getfilepath("themes"));
	QList<QString> fl = tpath->entryList(QDir::Dirs, QDir::Name);
	themeBox->insertItem(0, "Default");
	themeBox->setCurrentText("Default");
	themeBox->setItemData(0, "Default");
	for(QList<QString>::iterator it = fl.begin(); it != fl.end(); ++it){
		QString fname = *it;
		if(fname != "." || fname != ".."){
			QString p = tpath->path() + "/" + fname +"/"+fname+".theme.css";
			if(QFile::exists(p)){
				qDebug() << "Theme found: " + fname;
				themeBox->addItem(fname);
			}
		}
	}

	if(!config->getVal("theme").isEmpty() && !isWhiteSpace(config->getVal("theme"))){
		qDebug() << "Set theme:" << config->getVal("theme");
		QString themepath = tpath->path() + "/" + config->getVal("theme")+"/"+config->getVal("theme")+".theme.css";
		setStyleSheet(importStyle(themepath));
		themeBox->setCurrentText(config->getVal("theme"));
	}

	nickname->setText(config->getVal("nickname"));
	Qt::CheckState cs = ifc ? Qt::Checked : Qt::Unchecked;
	fabricb->setCheckState(cs);
	appshow();
}

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication app(argc, argv);
	app.setApplicationName("yokaiLauncher");
	app.setApplicationVersion("0.1.3"); // 0 - is beta
	QCommandLineParser p;
	p.addVersionOption();
	QCommandLineOption isDebug(QStringList() << "d" << "debug");
	p.addOption(isDebug);
	p.process(app);
	MWin n;
	n.debug = p.isSet(isDebug);
	app.setStyleSheet(n.importStyle(":/assets/default.qss"));
	app.setDesktopFileName("xyz.vilafox.mc.yokaiLauncher");
	app.setWindowIcon(QIcon(":/assets/icon.svg"));
	importFonts();
	app.setActiveWindow(&n);
	n.progstate = PState::INIT;
	n.loadconf();
	return app.exec();
}
