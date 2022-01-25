#ifndef MA_SETTINGS_H
#define MA_SETTINGS_H

#include <QtCore>
#include "dcmtk/dcmdata/dctypes.h"


#define SETTINGS_MODE_BLACKLIST 0
#define SETTINGS_MODE_WHITELIST 1

#define COMMAND_ID_KEEP   "keep"
#define COMMAND_ID_REMOVE "remove"
#define COMMAND_ID_CLEAR  "clear"
#define COMMAND_ID_SET    "set"

#define SET_MACRO_PROJECT_NAME "@project_name@"
#define SET_MACRO_PROJECT_OWNER "@project_owner@"
#define SET_MACRO_PROCESS_DATE "@process_date@"
#define SET_MACRO_RANDOM_UID "@random_uid@"
#define SET_MACRO_FAKE_NAME "@fake_name@"
#define SET_MACRO_FAKE_MRN "@fake_mrn@"
#define SET_MACRO_FAKE_ACC "@fake_acc@"
#define SET_MACRO_VALUE "@value@"

#define PRESET_DEFAULT "default"
#define PRESET_NONE "none"


class TagEntry 
{
public:
    enum Command {
        KEEP = 0,
        REMOVE,
        CLEAR,
        SET 
    };

    TagEntry()
    {
        set(0, 0, KEEP, "", "Unknown");
    }

    TagEntry(Uint16 _group, Uint16 _element, Command _command, QString _parameter, QString _source)
    {
        set(_group, _element, _command, _parameter, _source);
    }    

    void set(Uint16 _group, Uint16 _element, Command _command, QString _parameter, QString _source)
    {
        group=_group;
        element=_element;
        command=_command;
        parameter=_parameter;
        source=_source;
    }

    QString getCommandName() const
    {
        switch (command)
        {
        case KEEP:
            return QString(COMMAND_ID_KEEP);
        case REMOVE:
            return QString(COMMAND_ID_REMOVE);
        case CLEAR:
            return QString(COMMAND_ID_CLEAR);
        case SET:
            return QString(COMMAND_ID_SET);
        default:
            return "ERROR";
        }
    }

    void replaceParameterMacros();
    void replaceMacro(QString macro, QString value);

    Uint16 group;
    Uint16 element;
    Command command;
    QString parameter;
    QString source;
};


class ModuleSettings 
{
public:
    enum Preset {
        DEFAULT = 0,
        NONE
    };

    ModuleSettings() 
    {
        mode=SETTINGS_MODE_BLACKLIST;
        currentProject="";
        isPrepared=false;
        selectedPreset=DEFAULT;
    }

    int mode;
    bool isPrepared;
    QString currentProject;
    QString projectOwner;
    QString projectName;  
    Preset selectedPreset;
    QMap<QString, TagEntry> tags;
   
    bool prepareSettings(QString projectID);
    bool addTag(QString key, QString value, QString source);

protected:
    void replaceAllParameterMacros();
    void printTags();

};


#endif
