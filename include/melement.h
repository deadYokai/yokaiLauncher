#include <QWidget>
#include <QPixmap>

class MElement : public QWidget
{

public:
    explicit MElement(QWidget *parent = nullptr);
    ~MElement();
    enum modLoader { None, Fabric, Forge, Quilt };
    enum state { NoInstalled, Downloading, Installed };

    void setDescription(QString* d);
    void setModName(QString* m);
    void setIcon(QPixmap* p);

private slots:
    void clicked();

private:
    QWidget* _element;
    QString description;
    QString modname;
    QPixmap image;
    Ui::melement *ui;

};
