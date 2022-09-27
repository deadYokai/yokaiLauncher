#include <config_manager.h>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <fstream>

CMan::CMan(){}

void CMan::load(QString pathtofile){
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
}

QString CMan::getVal(QString valname){
    QString out;
    if(valname == "_debug"){
        return config["yokaiLauncher"][valname.toUtf8().constData()].as<QString>();
    }else{
        try{
            out = config["yokaiLauncher"][valname.toUtf8().constData()].as<QString>();
        }catch(const std::exception& e){
            QFile cfil(":/assets/defconf.yml");
            cfil.open(QFile::ReadOnly | QFile::Text);
            cfil.copy(".tmpdef.yml");
            YAML::Node n = YAML::LoadFile(".tmpdef.yml");
            out = n["yokaiLauncher"][valname.toUtf8().constData()].as<QString>();
            cfil.close();
            QFile::remove(".tmpdef.yml");
        }
        return out;
    }
}

void CMan::saveConf(){
    std::cout << "Writing config..." << std::endl;
    QString conf = "yokaiLauncher:\n  ";
    YAML::Node a = config["yokaiLauncher"];
    for (YAML::const_iterator it=a.begin();it!=a.end();++it) {
        QString key = it->first.as<QString>();
        QString val = it->second.as<QString>();
        conf.append(key + ": \"" + val + "\"\n  ");
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

