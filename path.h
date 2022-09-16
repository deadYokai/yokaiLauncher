#include <QDir>
#include <QString>

static struct
{
    QString mcPath = QDir::homePath() + "/.yokai";
    const QString verPath = "/versions/";
    const QString libsPath = "/libraries/";
    const QString assPath = "/assets/";
} Path;