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

class MWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit MWin(QWidget *parent = nullptr);
    void appshow();
    void disableControls(bool);
    void changeProgressState(int, QString, bool, bool);
    void changeProgressState(bool);
    QComboBox *vList;

private slots:
    void on_playBtn_clicked();


private:
    QWidget *ui_mw;
    QPushButton *playBtn;
    QWidget *mwCW;
    QMainWindow *mm;
    QLineEdit *nickname;
    QLabel *cNick;
    QLabel *pLabel;
    QWidget *pWidget;
    QProgressBar *progressBar;
};
