#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class MWin : public QWidget
{
    Q_OBJECT

public:
    explicit MWin(QWidget *parent = nullptr);

private slots:
    void on_enterBtn_clicked();
    void on_playBtn_clicked();
    // void on_enterBtn_clicked();

private:
  QWidget *ui_mw;
  QPushButton *ui_enterBtn;
  QWidget *client_f;
  QWidget *mwCW;
  QMainWindow *mm;
  QLineEdit *nicknameBox;
  QLabel *cNick;
};
