#include <QUrl>
#include <QString>

enum class PState {
    INIT,
    MANDOWN,
    MANCHEKSUM,
    VERMANDOWN,
    ASSDOWN,
    LIBDOWN,
    READY2PLAY,
    MLDown,
    JAVAIN,
    QuiltPostDownload,
    FabricPostDownload
};


enum class ModLoader { None, Quilt, Fabric, Forge};

class MLMaven{
    public:
        MLMaven(QUrl uri, QString path){
            MLMaven::uri = uri;
            MLMaven::path = path;
        }

        QUrl uri;
        QString path;
};
