#include <QWidget>
#include <QMainWindow>
#include <QPushButton>

class MWin : public QWidget
{
    Q_OBJECT

public:
    explicit MWin(QWidget *parent = nullptr);

private slots:
    void on_enterBtn_clicked();

private:
  QWidget *ui_mw;
  QPushButton *ui_enterBtn;
  QWidget *client_f;
  QWidget *mwCW;
  QMainWindow *mm;
//     QTextEdit *ui_textEdit;
//     QLineEdit *ui_lineEdit;

};
