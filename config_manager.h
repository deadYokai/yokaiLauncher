
#include <path.h>
#include <qtyaml.h>
#include <iostream>

class CMan
{

    public:
        CMan();
        
        void load(QString pathtofile);
        void saveConf();

        void setVal(QString key, QString val);
        void setVal(QString key, int val);
        void setVal(QString key, double val);

        QString getVal(QString valname);

    private:
        YAML::Node config;
        std::string path;

};