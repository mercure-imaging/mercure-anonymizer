#include <QtCore>

#define VERSION "0.1"
#define OUT(a) std::cout << a << std::endl;
#define TASKFILE "task.json"
#define SEPARATOR '#'

#define SETTINGS_MODE_BLACKLIST 0
#define SETTINGS_MODE_WHITELIST 1


class TagEntry 
{
public:
    TagEntry(QString _id, Uint16 _group, Uint16 _element, QString _setting) 
    {
        id=_id;
        group=_group;
        element=_element;
        setting=_setting;
    }

    QString id;
    Uint16 group;
    Uint16 element;
    QString setting;
};


class ModuleSettings 
{
public: 
    ModuleSettings() 
    {
        mode=SETTINGS_MODE_BLACKLIST;
    }

    int mode;
    QList<TagEntry> tags;
};


ModuleSettings settings;
QJsonObject settingsJson;

QString inputFolder = "";
QString outputFolder = "";

QDir inputDir;
QDir outputDir;

QFileInfoList inputFiles;

