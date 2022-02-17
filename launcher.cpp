
#include <launcher.h>
#include <QApplication>
#include <QFile>
#include <QUiLoader>
#include <QFontDatabase>
#include <QScreen>
#include <QDebug>
#include <QObject>
#include <QVBoxLayout>

using namespace std;
static QWidget *loadUiFile(QString page, QWidget *parent = nullptr)
{
	QFile file(":/assets/" + page + ".ui");
	file.open(QIODevice::ReadOnly);

	QUiLoader loader;
	QWidget *k;
	if(parent == nullptr)	k = loader.load(&file);
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

MWin::MWin(QWidget *parent) : QWidget(parent)
{
	ui_mw = loadUiFile("main", this);
	ui_enterBtn = findChild<QPushButton*>("enterBtn");
	mm = findChild<QMainWindow*>("MainWindow");
	mwCW = findChild<QWidget*>("centralwidget");
	client_f = loadUiFile("client", this);
	cNick = client_f->findChild<QLabel*>("cNick");
	nicknameBox = findChild<QLineEdit*>("nicknameBox");
	ui_mw->show();
	client_f->hide();
	QMetaObject::connectSlotsByName( this );

}

void MWin::on_enterBtn_clicked(){
	MWin::client_f->show();
	MWin::mm->setCentralWidget(MWin::client_f);
	MWin::cNick->setText(MWin::nicknameBox->text());
}

void MWin::on_playBtn_clicked(){
}

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication app(argc, argv);
	app.setStyleSheet(importQss());
	importFonts();
	MWin n;
	return app.exec();
}
