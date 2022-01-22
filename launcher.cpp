
#include <launcher.h>
#include <QApplication>
#include <QFile>
#include <QUiLoader>
#include <QFontDatabase>
#include <QScreen>

using namespace std;

static QWidget *loadUiFile()
{
	QFile file(":/assets/main.ui");
	file.open(QIODevice::ReadOnly);

	QUiLoader loader;
	return loader.load(&file);
}

QString importQss(){
	QFile file(":/assets/style.qss");
	file.open(QFile::ReadOnly | QFile::Text);
	QString styleSheet = QLatin1String(file.readAll());

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

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication app(argc, argv);
	app.setStyleSheet(importQss());
	importFonts();
	QWidget *n = loadUiFile();
	n->move(QGuiApplication::screens().at(0)->geometry().center() - n->frameGeometry().center());
	n->show();
	return app.exec();
}
