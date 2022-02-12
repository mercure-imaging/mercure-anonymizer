#ifndef MA_SETTINGS_H
#define MA_SETTINGS_H

#include <QtCore>
#include "dcmtk/dcmdata/dctypes.h"


#define SETTINGS_MODE_BLACKLIST 0
#define SETTINGS_MODE_WHITELIST 1

#define COMMAND_ID_KEEP "keep"
#define COMMAND_ID_SAFE "safe"
#define COMMAND_ID_REMOVE "remove"
#define COMMAND_ID_CLEAR "clear"
#define COMMAND_ID_SET "set"
#define COMMAND_ID_TRUNCDATE "truncdate"

#define SET_MACRO_PROJECT_NAME "@project_name@"
#define SET_MACRO_PROJECT_OWNER "@project_owner@"
#define SET_MACRO_PROJECT_PREFIX "@project_prefix@"
#define SET_MACRO_PROCESS_DATE "@process_date@"
#define SET_MACRO_PROCESS_TIME "@process_time@"
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
        SAFE,
        REMOVE,
        CLEAR,
        SET,
        TRUNCDATE
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
        case SAFE:
            return QString(COMMAND_ID_SAFE);
        case REMOVE:
            return QString(COMMAND_ID_REMOVE);
        case CLEAR:
            return QString(COMMAND_ID_CLEAR);
        case SET:
            return QString(COMMAND_ID_SET);
        case TRUNCDATE:
            return QString(COMMAND_ID_TRUNCDATE);
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
        isPrepared=false;
        mode=SETTINGS_MODE_BLACKLIST;
        currentProject="";
        selectedPreset=DEFAULT;
        projectOwner="";
        projectName="";
        projectPrefix="";      
        removeUnknownTags=false;
        removeSafeTags=false;
        removeCurves = false;
        removeOverlays = false;
    }

    bool isPrepared;

    int mode;
    QString currentProject;
    QString projectOwner;
    QString projectName;
    QString projectPrefix;
    Preset selectedPreset;
    QMap<QString, TagEntry> tags;

    bool removeUnknownTags;
    bool removeSafeTags;
    bool removeCurves;
    bool removeOverlays;
   
    bool prepareSettings(QString projectID);
    bool addTag(QString key, QString value, QString source);

protected:
    void replaceAllParameterMacros();
    void printTags();

};


#endif
