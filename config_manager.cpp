#include <config_manager.h>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <fstream>

CMan::CMan(){}
CMan::CMan(QString pathtofile){
    path = pathtofile.toUtf8().constData();
	if(!QFile::exists(pathtofile)){
        QFile cfile(":/assets/defconf.yml");
        cfile.open(QFile::ReadOnly | QFile::Text);
		std::cout << "Writing default config" << std::endl;
		if(cfile.copy(pathtofile)){
            QFile::setPermissions(pathtofile, QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::WriteGroup | QFileDevice::ReadGroup | QFileDevice::WriteUser | QFileDevice::ReadUser | QFileDevice::WriteOwner | QFileDevice::ReadOwner);
            cfile.close();
            config = YAML::LoadFile(path);
            setVal("mcdir", QDir::homePath() + "/.yokai");
        }
	}else
        config = YAML::LoadFile(path);
    setVal("mcdir", QDir::homePath() + "/.yokai");
}

QString CMan::getVal(QString valname){
    return config["yokaiLauncher"][valname.toUtf8().constData()].as<QString>();
}

void CMan::saveConf(){
    std::cout << "Writing config..." << std::endl;
    QString conf = "yokaiLauncher:\n\t";
    YAML::Node a = config["yokaiLauncher"];
    for (YAML::const_iterator it=a.begin();it!=a.end();++it) {
        QString key = it->first.as<QString>();
        QString val = it->second.as<QString>();
        conf.append(key + ": \"" + val + "\"\n\t");
    }

    QFile outf(QString::fromStdString(path));
    outf.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream qts(&outf);
    qts << conf;
    outf.close();
}

void CMan::setVal(QString key, QString val){
    config["yokaiLauncher"][key.toUtf8().constData()] = val.toUtf8().constData();
    saveConf();
}

void CMan::setVal(QString key, int val){
    config["yokaiLauncher"][key.toUtf8().constData()] = val;
    saveConf();
}

void CMan::setVal(QString key, double val){
    config["yokaiLauncher"][key.toUtf8().constData()] = val;
    saveConf();
}

