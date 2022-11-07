
#include <path.h>
#include <qtyaml.h>
#include <QString>
#include <QSettings>
class CMan
{

    public:
        CMan();
        
	    QSettings settings;
    
        void load(QString pathtofile);
        void saveConf();

        void setVal(QString key, QString val);
        void setVal(QString key, int val);
        void setVal(QString key, double val);

        QString getVal(QString valname);

    private:
        YAML::Node config;
        QString path;

};
