#ifndef MA_SETTINGS_H
#define MA_SETTINGS_H

#include <QtCore>

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
        currentProject="";
        isPrepared=false;
    }

    int mode;
    QList<TagEntry> tags;
    QString currentProject;
    bool isPrepared;

    void prepareSettings(QString projectID);
};


#endif
