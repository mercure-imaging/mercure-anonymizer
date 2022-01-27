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


#endif

