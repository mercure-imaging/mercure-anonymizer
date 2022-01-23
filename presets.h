#ifndef MA_PRESETS_H
#define MA_PRESETS_H

#include "global.h"
#include "settings.h"
#include "runtime.h"


class Presets
{
public:
    static bool addAssignments();

protected:
    static bool addAssignmentsDefault();
    static bool addTag(QString key, QString value);

};


inline bool Presets::addTag(QString key, QString value)
{
    return RTI->settings.addTag(key, value, "preset");
}


inline bool Presets::addAssignments()
{
    switch (RTI->settings.selectedPreset)
    {
    case ModuleSettings::DEFAULT:
        OUT("-- Using default preset assignment")
        return addAssignmentsDefault();
        break;
    case ModuleSettings::NONE:
        OUT("-- Skipping preset assignment")
        return true;
        break;
    default:
        OUT("ERROR: Invalid preset encountered! Aborting")
        return false;
    }
}


inline bool Presets::addAssignmentsDefault()
{
    bool res = true;
    res &= addTag("(0010,1040)", "remove");
    res &= addTag("(0010,1043)", "remove");
    return res;
}

#endif

