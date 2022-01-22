#include <QWidget>
#include <QMainWindow>

class MWin : public QWidget
{
    Q_OBJECT

public:
    explicit MWin(QWidget *parent = nullptr);

// private slots:
//     void on_findButton_clicked();

private:
  QMainWindow *ui_mw;
//     QPushButton *ui_findButton;
//     QTextEdit *ui_textEdit;
//     QLineEdit *ui_lineEdit;
};
