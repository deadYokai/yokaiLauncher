#include <launcher.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <future>

#if _WIN32 || _WIN64
#include <QWinTaskbarButton>
#endif

#define CURL_STATICLIB
#include <curl/curl.h>

using json = nlohmann::json;
using namespace std;

json versionData;
string latestVersion;


void dp(string a){cout << a << endl;}
void dp(char a){cout << a << endl;}
void dp(int a){cout << a << endl;}

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

void MWin::changeProgressState(bool show){
	int mH = show ? 40 : 0;
	pWidget->setMaximumHeight(mH);
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

json purl(){
	CURL *curl;
	CURLcode res;
	string readBuffer;

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "https://vilafox.xyz/api/yokaiLauncher?get=release");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return json::parse(readBuffer);
}

void MWin::on_playBtn_clicked(){
	changeProgressState(100, "Play button clicked");
	dp("Play button clicked");
}

// void aFunction(QPromise<int> &promise)
// {
//     promise.setProgressRange(0, 100);
//     int result = 0;
//     for (int i = 0; i < 100; ++i) {
//         // computes some part of the task
//         const int part = ... ;
//         result += part;
//         promise.setProgressValue(i);
//     }
//     promise.addResult(result);
// }
// QFutureWatcher<int> watcher;
// QObject::connect(&watcher, &QFutureWatcher::progressValueChanged, [](int progress){
//     ... ; // update GUI with a progress
//     qDebug() << "current progress:" << progress;
// });
// watcher.setFuture(QtConcurrent::run(aFunction));
void st(MWin* n){
	dp("Getting version manifest...");
	future<json> manRes = async(purl);
	versionData = manRes.get();
	latestVersion = versionData["latest"];
	dp("Latest version: " + latestVersion);
	for (json::iterator it = versionData["versionlist"].begin(); it != versionData["versionlist"].end(); ++it) {
		n->vList->addItem(QString::fromStdString(*it));
	}
	n->vList->setCurrentIndex(0);
	n->changeProgressState(false);
	n->disableControls(false);
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
	changeProgressState(0, "Getting version manifest...", false);
	auto future = QtConcurrent::run(st, this);
	
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
	n.appshow();
	return app.exec();
}
