
#include <ui_melement.h>
#include <ui_dcon.h>
#include <ui_client.h>
#include <launcher.h>
#include <QGraphicsBlurEffect>
#include <fstream>
#include <QDialog>
#include <limits>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include <dirdialog.h>

// Custom debug outputs (idk why i'm need this)
void dp(QString a){	qDebug() << a;}
void dp(char a){qDebug() << a;}
void dp(int a){qDebug() << a;}
void dp(double a){qDebug() << a; }

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
				_pixmapBg.load(":/assets/bg.png");
			}
		}else{
			qDebug() << "Warn: Invalid background image, setting to default";
			_pixmapBg.load(":/assets/bg.png");
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


	// pWidget->hide();
	// mwCW->hide();
	// bwi->hide();
	// uwi->show();
}

void MWin::disableControls(bool a = true){
	bool val = !a;
	nickname->setEnabled(val);
	vList->setEnabled(val);
	playBtn->setEnabled(val);
	fabricb->setEnabled(val);
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


// UI & Events Init
MWin::MWin(QWidget *parent) : QMainWindow(parent), ui(new Ui::MWin)
{

	ui->setupUi(this);
	this->setWindowFlags(Qt::Window);
	this->setWindowModality(Qt::ApplicationModal);
	
	ui_mw = ui->ClientForm;
	mwCW = ui->Logo;
	nickname = ui->nickname;
	pWidget = ui->progressWidget;
	vList = ui->versionList;
	pLabel = ui->pLabel;
	progressBar = ui->progressBar;
	playBtn = ui->playBtn;
	fabricb = ui->fabricb;
	
	themeBox = ui->themesBox;
	mcPathEdit = ui->mcPathEdit;
	mcPathSel = ui->mcPathSelect;
	mcFolBtn = ui->mcFolderBtn;

	bid = new QLabel(ui_mw);
	bid->setStyleSheet("font-size: 14px");
	bid->setGeometry(8, 8, 1000, 32);
	settsavebtn = ui->settsaveb;
	bid->setText("Build #" + QString::number(BUILDID));

	ui->verLab->setText(QString::number(_VERSION));

	ui->stackedWidget->setMaximumWidth(0);

	connect(ui->settb, &QPushButton::clicked, this, &MWin::pageBtnClick);
	connect(ui->modsBtn, &QPushButton::clicked, this, &MWin::pageBtnClick);
	connect(ui->modpackBtn, &QPushButton::clicked, this, &MWin::pageBtnClick);
	connect(ui->modloaderBtn, &QPushButton::clicked, this, &MWin::pageBtnClick);

	isf = ui->isMcFullscreen;
	jvma = ui->jvmaddargs;
	rams = ui->ramslider;
	mml = ui->maxmemLabel;
	cram = ui->currRam;

	connect(vList, &QComboBox::currentTextChanged, this, &MWin::verChanged);
	connect(fabricb, &QCheckBox::stateChanged, this, &MWin::isFabricbox);

	connect(mcFolBtn, &QPushButton::clicked, this, [=](){ QDesktopServices::openUrl(QUrl("file:///"+config->getVal("mcdir"), QUrl::TolerantMode)); });
	connect(playBtn, &QPushButton::clicked, this, [=](){
		disableControls();
		config->setVal("nickname", nickname->text());
		QString dpath = Path.verPath + vList->currentText();
		QString manifestUri = vData.value(vList->currentText());
		QDir d;
		QList<MLMaven*> files;
		if(d.mkpath(getfilepath(dpath))){
			progstate = PState::VERMANDOWN;
			currManFile = dpath + "/" + vList->currentText() + ".json";
			if(!QFile::exists(getfilepath(dpath + "/" + vList->currentText() + ".json"))){
				files.append(new MLMaven(QUrl(manifestUri), dpath + "/" + vList->currentText() + ".json"));
				dwF(files);
			}else
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
				config->settings.setValue("mcpath", QDir::cleanPath(dir->selectedFiles()[0]));
				mcPathEdit->setText(dir->selectedFiles()[0]);
			}
		}
		// TODO
		// ChooseDirDialog *cd = new ChooseDirDialog(ui_mw);
		// cd->open(Path.mcPath);
		// QString path = cd->getPathStr();
		// if(!path.isEmpty()){
		// 	mcPathEdit->setText(path);
		// }
	 });
	connect(settsavebtn, &QPushButton::clicked, this, [=](){
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
		msgBox("Saved");
	});

	connect(&manager, &QNetworkAccessManager::finished,
            this, &MWin::downloadFinished);

}


void MWin::pageBtnClick(){
	int index = sender()->property("menuPage").toInt();

	if(ui->stackedWidget->width() != 0 && index == ui->stackedWidget->currentIndex()){
		ui->stackedWidget->setMaximumWidth(0);
	}else{
		ui->stackedWidget->setMaximumWidth(512);
	}

	ui->stackedWidget->setCurrentIndex(index);
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
	if(run) process->kill();
    qDebug() << "Bye...";
}

MWin::~MWin(){
	if(!isMaximized()){
		config->setVal("height", height());
		config->setVal("width", width());
	}
	config->setVal("maximized", isMaximized());
	dp("===============================\n");

	delete ui;
}

void MWin::changeProgressState(int progress, QString text, bool showBar = true, bool show = true){
	pWidget->setVisible(show);
	int mH = show ? 40 : 0;
	pWidget->setMaximumHeight(mH);
	int pHeight = showBar ? 32 : 0;
	progressBar->setMaximumHeight(pHeight);
	progressBar->setValue(progress);
	pLabel->setText(text);
}

void MWin::changeProgressState(int progress, int max, QString text, bool showBar = true, bool show = true){
	pWidget->setVisible(show);
	int mH = show ? 40 : 0;
	pWidget->setMaximumHeight(mH);
	int pHeight = showBar ? 32 : 0;
	progressBar->setMaximumHeight(pHeight);
	progressBar->setMaximum(max);
	progressBar->setValue(progress);
	pLabel->setText(text);
}

void MWin::changeProgressState(bool show){
	pWidget->setVisible(show);
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

	switch(cML){
		case ModLoader::Fabric:
			args.append(" -DFabricMcEmu="+currmanj["mainClass"].toString());
			break;
		case ModLoader::Quilt:
			break;
		default:
			break;
	}

	QString mainclass = ( cML != ModLoader::None ) ? MLMclass : currmanj["mainClass"].toString();
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
	
	if(cML != ModLoader::None) libs.append(MLLibs);
	
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

QList<MLMaven*> MWin::getMavenData(QJsonArray ja){
	QList<MLMaven*> list;
	for(QJsonArray::iterator it = ja.begin(); it != ja.end(); ++it){
		QJsonValue a = *it;
		QJsonObject jo = a.toObject();
		QStringList libMaven = jo["name"].toString().split(":");
		QString mavenUri = jo["url"].toString();
		QString apath = libMaven[0].replace(".", "/");
		QString name = libMaven[1];
		QString ver = libMaven[2];
		QString qpath = apath + "/" + name + "/" + ver + "/" + name + "-" + ver + ".jar";
		QUrl libUrl = QUrl(mavenUri + qpath);
		if(!QFile::exists(getfilepath(Path.libsPath + qpath))){
			list.append(new MLMaven(libUrl, Path.libsPath + qpath));
		}
		QString paths = ":";
		#ifdef Q_OS_WIN
		paths = ";";
		#endif
		MLLibs.append(getfilepath(Path.libsPath + qpath) + paths);
	}
	return list;
}

void MWin::quiltDownload(){
	QString loaderMaven = "https://maven.quiltmc.org/repository/release/";
	QString fabricMaven = "https://maven.fabricmc.net/";

	QString fabricMcMavenDir = "net/fabricmc/";
	QString McMavenDir = "org/quiltmc/";
	QString MavenDir = McMavenDir + "quilt-loader/";
	QString fp = ".quilt/quilt-loader-"+quiltVer;
	QString path = fp +".json";
	QString fab = loaderMaven+MavenDir+quiltVer+"/quilt-loader-"+quiltVer;
	progstate = PState::MLDown;
	QList<MLMaven*> files;
	if(!QFile::exists(getfilepath(path))){
		QFileInfo fi(getfilepath(path));
		QDir dir(fi.dir().path());
		dir.setNameFilters(QStringList() << "quilt-loader-*.*\\.jar" << "quilt-loader-*.*\\.json");
		dir.setFilter(QDir::Files);
		foreach(QString dirFile, dir.entryList())
		{
			dir.remove(dirFile);
		}
		files.append(new MLMaven(fab+".json", path));
	}
	
	if(!QFile::exists(getfilepath(fp+".jar"))){
		currFile = getfilepath(fp+".jar");
		files.append(new MLMaven(fab+".jar", fp+".jar"));
	}


	QString qpath = fabricMcMavenDir + "/intermediary/" + currmanj["id"].toString() + "/intermediary-" + currmanj["id"].toString() + "-v2.jar";
	QString libPath = Path.libsPath + qpath;

	if(!QFile::exists(getfilepath(libPath))){
		currFile = getfilepath(libPath);
		files.append(new MLMaven(fabricMaven + qpath, libPath));
	}

	progstate = PState::QuiltPostDownload;
	dwF(files);
	
}

void MWin::quiltpost(){
	
	QString fp = ".quilt/quilt-loader-"+quiltVer;
	QString path = fp +".json";
	QList<MLMaven*> files;
	QFile f(getfilepath(path));
	if (!f.open(QFile::ReadOnly | QFile::Text)) return;
	QTextStream in(&f);
	QString str = in.readAll();
	QJsonDocument jsonResponse = QJsonDocument::fromJson(str.toUtf8());
	QJsonArray ja = jsonResponse.object()["libraries"].toObject()["common"].toArray();
	MLMclass = jsonResponse.object()["mainClass"].toObject()["client"].toString();
	QString paths = ":";
	#ifdef Q_OS_WIN
	paths = ";";
	#endif

	files = getMavenData(ja);

	QString fabricMcMavenDir = "net/fabricmc/";
	QString qpath = fabricMcMavenDir + "/intermediary/" + currmanj["id"].toString() + "/intermediary-" + currmanj["id"].toString() + "-v2.jar";
	QString libPath = Path.libsPath + qpath;

	MLLibs.append(getfilepath(libPath)+paths);
	MLLibs.append(getfilepath(fp + ".jar")+paths);
	progstate = PState::READY2PLAY;
	dwF(files);
}

// Check Fabric and Download
void MWin::fabricDownload(){
	QString fabricMaven = "https://maven.fabricmc.net/";
	QString fabricMcMavenDir = "net/fabricmc/";
	QString fabricMavenDir = fabricMcMavenDir + "fabric-loader/";
	QString fp = ".fabric/fabric-loader-"+fabVer;
	QString path = fp +".json";
	QString fab = fabricMaven+fabricMavenDir+fabVer+"/fabric-loader-"+fabVer;
	progstate = PState::MLDown;
	if(!QFile::exists(getfilepath(path))){
		QFileInfo fi(getfilepath(path));
		QDir dir(fi.dir().path());
		dir.setNameFilters(QStringList() << "fabric-loader-*.*\\.jar" << "fabric-loader-*.*\\.json");
		dir.setFilter(QDir::Files);
		foreach(QString dirFile, dir.entryList())
		{
			dir.remove(dirFile);
		}
		// downloadFile(fab+".json", path);
		return;
	}

	if(!QFile::exists(getfilepath(fp+".jar"))){
		currFile = getfilepath(fp+".jar");
		// downloadFile(fab+".jar", fp+".jar");
		return;
	}

	QFile f(getfilepath(path));
	if (!f.open(QFile::ReadOnly | QFile::Text)) return;
	QTextStream in(&f);
	QString str = in.readAll();
	QJsonDocument jsonResponse = QJsonDocument::fromJson(str.toUtf8());
	QJsonArray ja = jsonResponse.object()["libraries"].toObject()["common"].toArray();
	MLMclass = jsonResponse.object()["mainClass"].toObject()["client"].toString();
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
		MLLibs.append(getfilepath(libPath)+paths);
		QUrl libUrl = QUrl(fabricMaven + qpath);
		currFile = libPath;
		if(!QFile::exists(getfilepath(libPath))){
			// downloadFile(libUrl, libPath);
			return;
		}
	}
	

	QString qpath = fabricMcMavenDir + "/intermediary/" + currmanj["id"].toString() + "/intermediary-" + currmanj["id"].toString() + "-v2.jar";
	QString libPath = Path.libsPath + qpath;

	if(!QFile::exists(getfilepath(libPath))){
		currFile = getfilepath(libPath);
		// downloadFile(fabricMaven + qpath, libPath);
		return;
	}

	dp("Fabric enabled");
	MLLibs.append(getfilepath(libPath)+paths);
	MLLibs.append(getfilepath(fp + ".jar")+paths);
	checkJava();
}

// Check Java and Downloads
void MWin::checkJava(){
	javaRuntimeDir = QString::fromLocal8Bit(qgetenv("JAVA_HOME"));
	qDebug() << "Java init";
#ifdef Q_OS_WIN
	
	QList<MLMaven*> files;
	QUrl jmanifest = QUrl("https://launchermeta.mojang.com/v1/products/java-runtime/2ec0cc96c44e5a76b9c8b7c39df7210883d12871/all.json");
	QString winp = "x86";
	#ifdef Q_OS_WIN64
		winp = "x64";
	#endif
	progstate = PState::JAVAIN;

	if(!QFile::exists(getfilepath("java/all.json"))){
		currFile = getfilepath("java/all.json");
		// downloadFile(jmanifest, "java/all.json");
		files.append(new MLMaven(jmanifest, getfilepath("java/all.json")));
		dwF(files);

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
		// downloadFile(runtimeurl, "java/"+runtimever+"/windows-"+winp+".json");

		files.append(new MLMaven(runtimeurl, "java/"+runtimever+"/windows-"+winp+".json"));
		dwF(files);
		f.close();
		return;
	}

	QFile f(getfilepath("java/"+runtimever+"/windows-"+winp+".json"));
	if (!f.open(QFile::ReadOnly | QFile::Text)) return;
	QTextStream in(&f);
	QString str = in.readAll();
	QJsonDocument jsonResponse = QJsonDocument::fromJson(str.toUtf8());
	QJsonObject jfiles = jsonResponse.object()["files"].toObject();
	QStringList keys = jfiles.keys();
	for (QStringList::iterator it = keys.begin(); it != keys.end(); ++it) {
		QString b = *it;
		QJsonObject a = jfiles[b].toObject();
		if(a["type"] == "file"){
			if(!QFile::exists(getfilepath("java/"+runtimever+"/windows-"+winp+"/"+b))){
				QUrl uri = QUrl(a["downloads"].toObject()["raw"].toObject()["url"].toString());
				currFile = getfilepath("java/"+runtimever+"/windows-"+winp+"/"+b);
				// downloadFile(uri, "java/"+runtimever+"/windows-"+winp+"/"+b);
				
				files.append(new MLMaven(uri, "java/"+runtimever+"/windows-"+winp+"/"+b));
				dwF(files);
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
		QPlainTextEdit *pp = dcon->debugT;
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
	QPlainTextEdit *pp = dcon->debugT;
	while(p->canReadLine())
	{
		pp->appendPlainText(p->readLine().replace("\n", ""));
	}

}

// Write Minecraft output to DebugLog
void MWin::rr(){
	QProcess *p = qobject_cast<QProcess*>(sender());
	p->setReadChannel(QProcess::StandardOutput);
	QPlainTextEdit *pp = dcon->debugT;
	// pp->appendPlainText("Minecraft launching...");
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
		QPlainTextEdit *pp = dcon->debugT;
		pp->appendPlainText("Launching...");
	}
	connect(process, &QProcess::finished, this, &MWin::mcend);
	if(debug){
		connect(process, &QProcess::readyReadStandardOutput, this, &MWin::rr);
		connect(process, &QProcess::readyReadStandardError, this, &MWin::re);
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
			if(reply->error() == QNetworkReply::ContentNotFoundError && progstate == PState::MLDown){
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

	dwFiles.removeAll(reply);
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
	quiltVer = jsonObject["quilt"].toString();
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

void MWin::r2run(){
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
	cML = ModLoader::Quilt;
	switch(cML){
		case ModLoader::Fabric:
			fabricDownload();
			break;
		case ModLoader::Quilt:
			quiltDownload();
			break;
		default:
			checkJava();
	}
}

bool MWin::isWhiteSpace(const QString & str)
{
	return QRegularExpression("\\s.").match(str).hasMatch();
}

// Download assets for Minecrafts
void MWin::assdown(){
	QJsonObject ja = assmanj["objects"].toObject();
	QList<MLMaven*> files;
	for(QJsonObject::iterator it = ja.begin(); it != ja.end(); ++it){
		++ass;
		QJsonValue a = *it;
		QJsonObject jo = a.toObject();
		QString hash = jo["hash"].toString();
		QString chash = QString(hash.data()[0]) + QString(hash.data()[1]);
		QString apath = Path.assPath + "objects/" + chash + "/" + hash;
		QUrl aurl = QUrl("http://resources.download.minecraft.net/" + chash + "/" + hash);
		if(!QFile::exists(getfilepath(apath))){
			files.append(new MLMaven(aurl, apath));
		}

		// if(!checksha1(getfilepath(apath), hash)){
			// if(!files.contains(aurl)){
			// 	files.append(aurl);
			// 	pat.append(apath);
				// downloadFile(aurl, apath);
				// return;
			// }
		// }
	}

	dwF(files);

}

// Download Java libraries
void MWin::libdown(){
	QList<MLMaven*> files;
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
					files.append(new MLMaven(libUrl, libPath));
				}
				QString u = jo["downloads"].toObject()["classifiers"].toObject()["natives-"+p].toObject()["url"].toString();
				QString pp = Path.libsPath + jo["downloads"].toObject()["classifiers"].toObject()["natives-"+p].toObject()["path"].toString();
				if(!QFile::exists(getfilepath(pp))){
					files.append(new MLMaven(u, pp));
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
					files.append(new MLMaven(libUrl, libPath));
				}
			}else{
				files.append(new MLMaven(libUrl, libPath));
			}
		}

		// if(!checksha1(getfilepath(libPath), libSha1)){
		// 	downloadFile(libUrl, libPath);
		// 	return;
		// }
	}

	dwF(files);
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
	QList<MLMaven*> files;
	if(!QFile::exists(getfilepath(dpath))){
		files.append(new MLMaven(QUrl(currmanj["downloads"].toObject()["client"].toObject()["url"].toString()), dpath));
		dwF(files);
		return;
	}else{
		if(!QFile::exists(getfilepath(assPathFile))){
			files.append(new MLMaven(uri, assPathFile));
			dwF(files);
			return;
		}else{
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
			progstate = PState::ASSDOWN;
			assdown();
			break;
		case PState::ASSDOWN:
			r2run();
			break;
		case PState::READY2PLAY:
			checkJava();
			break;
		case PState::JAVAIN:
			checkJava();
			break;
		case PState::QuiltPostDownload:
			quiltpost();
			break;
		case PState::MLDown:
			switch(cML){
				case ModLoader::Fabric:
					fabricDownload();
					break;
				case ModLoader::Quilt:
					quiltDownload();
					break;
				case ModLoader::Forge:
					msgBox("Not supported, yet!");
					break;
				default:
					break;
			}
			break;
		default:
			changeProgressState(0, "Done.", false);
			disableControls(false);
	}
}

// Connect http request events
void MWin::httpReq(const QUrl &requestedUrl) {
	QNetworkRequest r(requestedUrl);
	r.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
	reply = qnam.get(r);
	if(progstate != PState::MANCHEKSUM){
		if(!m){
			connect(reply, &QIODevice::readyRead, this, &MWin::httpRead);
			connect(reply, &QNetworkReply::downloadProgress, this, &MWin::progress_func);
		}
		connect(reply, &QNetworkReply::finished, this, &MWin::httpFinish);
	}else{
		connect(reply, &QIODevice::readyRead, this, &MWin::getCheckSum);
	}

}

void MWin::doDownload(const QUrl &url, const QString &path = nullptr)
{
	changeProgressState(true);
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);
#if QT_CONFIG(ssl)
    connect(reply, &QNetworkReply::sslErrors,
            this, &MWin::sslErrors);
#endif
	dwMap[reply] = path;
    currentDownloads.append(reply);
}

void MWin::sslErrors(const QList<QSslError> &sslErrors)
{
#if QT_CONFIG(ssl)
    for (const QSslError &error : sslErrors)
        qDebug() << "SSL error: " << error.errorString();
#else
    Q_UNUSED(sslErrors);
#endif
}

bool MWin::saveToDisk(const QString &filename, QIODevice *data)
{
    QFile file(filename);
	QDir d;
	QFileInfo fi(filename);
	if(!d.mkpath(fi.dir().path()))
		return false;

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << filename << "::" << file.errorString();
        return false;
    }

    file.write(data->readAll());
    file.close();

    return true;
}

void MWin::downloadFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();
	QString filename = dwMap[reply];
    if (reply->error()) {
        qDebug() << reply->errorString();
        return;
    } else {
        if (isHttpRedirect(reply)) {
            fputs("Request was redirected.\n", stderr);
        } else {

            if (!saveToDisk(filename, reply)) {
                return;
            }
        }
    }
	
	dwMap.remove(reply);
    currentDownloads.removeAll(reply);
    reply->deleteLater();

	ass++;

	QString dwtext = "Downloaded ";

	// if(!currFile.isEmpty()) dwtext += currFile;

	changeProgressState(static_cast<int>(ass), static_cast<int>(assm), dwtext + QString::number(static_cast<int>(ass)) + "/" + QString::number(static_cast<int>(assm)));

	if(currentDownloads.isEmpty()){
		progressFinish();
	}

}

bool MWin::isHttpRedirect(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void MWin::dwF(QList<MLMaven*> files){
	ass = 0;
	assm = files.length();
	if(files.isEmpty()) progressFinish();
	for (QList<MLMaven*>::iterator it = files.begin(); it != files.end(); ++it) {
        MLMaven* m = *it;
        doDownload(m->uri, getfilepath(m->path));
    }
}

// Check manifest file
void MWin::purl(){
	changeProgressState(0, "Getting version manifest...", false);
	QString manpath = "yokaiLauncher_manifest.json";
	dp("Getting version manifest...");
	QList<MLMaven*> files;
	if(!QFile::exists(getfilepath(manpath))){
		progstate = PState::MANDOWN;
		currFile = manpath;
		dp("Downloading manifest...");
		files.append(new MLMaven(QUrl("https://vilafox.xyz/api/yokaiLauncher"), manpath));
		dwF(files);
	}else{	
		progstate = PState::MANCHEKSUM;
		progressFinish();
	}

}


// UI Checks
void MWin::appshow(){
	dp("UI loaded");

	if(debug){
		QWidget* w = new QWidget(ui_mw);
		dcon = new Ui::Form();
		dcon->setupUi(w);
		w->setWindowFlags(Qt::Window);
		w->show();
	}

	disableControls();
	purl();
	
}

// Config loader & Init app
void MWin::loadconf()
{
	dp("\n======== yokaiLauncher ========");
	dp("Init");
	config = new CMan();
	if(!config->settings.value("mcpath", "").toString().isEmpty()) Path.mcPath = config->settings.value("mcpath").toString();
	QString cpath = Path.mcPath + "/yokai.yml";
	QDir d;
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

	bool ifsc = QVariant(config->getVal("isFullscreen")).toBool();
	isf->setCheckState(ifsc ? Qt::Checked : Qt::Unchecked);

	jvma->setText(config->getVal("jvmargs"));

	uint64_t mbRam = getSystemRam() / (1024ul * 1024ul);
	rams->setMinimum(2048);
	if(mbRam >= 4096){
		config->setVal("ram", (int)mbRam/2);
	}else{
		rams->setMinimum(512);
		config->setVal("ram", 2048);
		msgBox("Warning: low RAM in your PC");
	}
	
	rams->setMaximum(mbRam);
	connect(rams, &QSlider::valueChanged, [=](int value){
		cram->setText("Current RAM: " + QString::number(value) + "MB");
	});

	mml->setText(QString::number(mbRam/1024) + "GB");
	cram->setText("Current RAM: " + QString::number(rams->value()) + "MB");

	appshow();
}

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication app(argc, argv);
	app.setApplicationName("yokaiLauncher");
	app.setApplicationVersion(QString::number(_VERSION)); // 0 - is beta
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
