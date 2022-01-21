#ifndef MA_SETTINGS_H
#define MA_SETTINGS_H

#include <QtCore>
#include "dcmtk/dcmdata/dctypes.h"

#define SETTINGS_MODE_BLACKLIST 0
#define SETTINGS_MODE_WHITELIST 1

#define COMMAND_ID_CLEAR  "clear"
#define COMMAND_ID_REMOVE "remove"
#define COMMAND_ID_KEEP   "keep"
#define COMMAND_ID_SET    "set"


class TagEntry 
{
public:
    enum Command {
        CLEAR = 0,
        REMOVE,
        KEEP,
        SET 
    };

    TagEntry(QString _id, Uint16 _group, Uint16 _element, Command _command, QString _parameter) 
    {
        id=_id;
        group=_group;
        element=_element;
        command=_command;
        paramter=_parameter;
    }    

    QString id;
    Uint16 group;
    Uint16 element;
    QString paramter;
    Command command;
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
    bool isPrepared;
    QList<TagEntry> tags;
    QString currentProject;
    QString projectOwner;
    QString projectName;
    QString dateString;

    void prepareSettings(QString projectID);

protected:

    bool addTag(QString key, QString value);

};


#endif
