#include <config_manager.h>

#include <QDir>
#include <QTextStream>
#include <QDebug>
// #include <fstream>
#include <QFileDialog>
CMan::CMan(){}

void CMan::load(QString pathtofile){
    path = QDir::cleanPath(pathtofile);
	if(!QFile::exists(pathtofile)){
        QFileDialog *dir = new QFileDialog();
		dir->setFileMode(QFileDialog::Directory);
		dir->setOption(QFileDialog::ShowDirsOnly);
		dir->setViewMode(QFileDialog::List);
		dir->setDirectory(path);
		if(dir->exec()){
			if(!dir->selectedFiles()[0].isEmpty()){
                path = QDir::cleanPath(dir->selectedFiles()[0] + "/yokai.yml");
                settings.setValue("mcpath", QDir::cleanPath(dir->selectedFiles()[0]));
                QFile cfile(":/assets/defconf.yml");
                cfile.open(QFile::ReadOnly | QFile::Text);
                // std::cout << "Writing default config" << std::endl;
                if(cfile.copy(path)){
                    QFile::setPermissions(path, QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::WriteGroup | QFileDevice::ReadGroup | QFileDevice::WriteUser | QFileDevice::ReadUser | QFileDevice::WriteOwner | QFileDevice::ReadOwner);
                    cfile.close();
                    config = YAML::LoadFile(path.toUtf8().constData());
                    Path.mcPath = QDir::cleanPath(dir->selectedFiles()[0]);
                    setVal("mcdir", QDir::cleanPath(dir->selectedFiles()[0]));
                }
			}
		}
	}else{
        config = YAML::LoadFile(path.toUtf8().constData());
    }
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
    // std::cout << "Writing config..." << std::endl;
    QString conf = "yokaiLauncher:\n  ";
    YAML::Node a = config["yokaiLauncher"];
    for (YAML::const_iterator it=a.begin();it!=a.end();++it) {
        QString key = it->first.as<QString>();
        QString val = it->second.as<QString>();
        conf.append(key + ": \"" + val + "\"\n  ");
    }

    QFile outf(path);
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

