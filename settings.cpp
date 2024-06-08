#include "global.h"
#include "settings.h"
#include "runtime.h"
#include "presets.h"


extern "C" 
{
    #include "external/siphash/siphash.h"
}

void TagEntry::replaceParameterMacros()
{
    // Check the parameter values for macros and replace with corresponding values
    replaceMacro(SET_MACRO_PROJECT_NAME, RTI->settings.projectName);
    replaceMacro(SET_MACRO_PROJECT_OWNER, RTI->settings.projectOwner);
    replaceMacro(SET_MACRO_PROJECT_PREFIX, RTI->settings.projectPrefix);
    replaceMacro(SET_MACRO_PROCESS_DATE, RTI->dateString);
    replaceMacro(SET_MACRO_PROCESS_TIME, RTI->timeString);
    replaceMacro(SET_MACRO_RANDOM_UID, RTI->randomUID);
    replaceMacro(SET_MACRO_FAKE_MRN, RTI->fakeMRN);    
    replaceMacro(SET_MACRO_FAKE_ACC, RTI->fakeACC); 
    replaceMacro(SET_MACRO_HASH_ACC, RTI->hashACC);        
    replaceMacro(SET_MACRO_FAKE_NAME, RTI->fakeName);    
}


void TagEntry::replaceMacro(QString macro, QString value)
{
    // Replace all occurrances of the macro in the parameter string
    while (parameter.indexOf(macro)>=0)
    {
        parameter.replace(parameter.indexOf(macro), macro.size(), value);
    }
}


bool ModuleSettings::calculateHashACC() 
{
    if (RTI->accUsedForHash.isEmpty()) {
        OUT("WARNING: No ACC given in DICOMs. Unable to calculate HashACC.")
    }
    if (projectOwner.isEmpty()) {
        OUT("WARNING: No Project Owner provided. Unable to calculate HashACC.")
    }
    if (projectName.isEmpty()) {
        OUT("WARNING: No Project Name provided. HashACC might not be unique.")
    }

    QString key = projectOwner.toLower().leftJustified(16, ' ');
    QString value = projectName.toLower() + "-" + RTI->accUsedForHash.toLower();

    int hashlen = 8;
    uint8_t output_buffer[hashlen] = {};
    uint8_t* output_pointer = output_buffer;
    siphash(value.toStdString().c_str(), value.length(), key.toStdString().c_str(), output_pointer, 8);

    RTI->hashACC="";
    for (int i = 0; i < hashlen; i++) {
        RTI->hashACC+=QString::number(output_buffer[hashlen-1-i], 16);
    }
    RTI->hashACC=RTI->hashACC.toUpper();
    OUT("-- Hash ACC is " + RTI->hashACC.toStdString())

    return true;
}


bool ModuleSettings::prepareSettings(QString projectID)
{
    OUT("")
    OUT("-- Receiving AET is " << projectID.toStdString())

    QString cleanProjectID = projectID;
    if (cleanProjectID.startsWith(AET_PREFIX))
    {
        // Chop the prefix from the front of the AET
        cleanProjectID.remove(0,QString(AET_PREFIX).length());
    }

    QJsonObject generalSettings = RTI->settingsJson.value("general").toObject();

    tags.clear();
    projectOwner = "Unknown";
    projectName = "Undefined";
    projectPrefix = "";
    selectedPreset = DEFAULT;
    removeSafeTags = false;
    removeUnknownTags = true;
    removeCurves = true;
    removeOverlays = true;
    printAssignment = false;

    // Read general settings
    if (generalSettings.contains("name")) 
    {
        projectName = generalSettings.value("name").toString();
    }
    if (generalSettings.contains("owner")) 
    {
        projectOwner = generalSettings.value("owner").toString();
    }
    if (generalSettings.contains("prefix")) 
    {
        projectOwner = generalSettings.value("prefix").toString();
    }
    if (generalSettings.contains("preset")) 
    {
        selectedPreset=DEFAULT;
        if (generalSettings.value("preset").toString().toLower()==PRESET_NONE)
        {
            selectedPreset = NONE;

            // If the preset is set to none, then don't remove unknown tags, as
            // this would remove all tags. Also keep overlays and curves, unless
            // explicitly specified
            removeSafeTags = false;
            removeUnknownTags = false;
            removeCurves = false;
            removeOverlays = false;
        }
    }
    if (generalSettings.contains("remove_unknown_tags")) 
    {
        if (generalSettings.value("remove_unknown_tags").toString().toLower()=="false")
        {
            removeUnknownTags=false;
        }
        else
        {
            removeUnknownTags=true;
        }
    }
    if (generalSettings.contains("remove_safe_tags")) 
    {
        if (generalSettings.value("remove_safe_tags").toString().toLower()=="false")
        {
            removeSafeTags=false;
        }
        else
        {
            removeSafeTags=true;
        }
    }
    if (generalSettings.contains("remove_curves")) 
    {
        if (generalSettings.value("remove_curves").toString().toLower()=="false")
        {
            removeCurves=false;
        }
        else
        {
            removeCurves=true;
        }
    }
    if (generalSettings.contains("remove_overlays")) 
    {
        if (generalSettings.value("remove_overlays").toString().toLower()=="false")
        {
            removeOverlays=false;
        }
        else
        {
            removeOverlays=true;
        }
    }
    if (generalSettings.contains("print_assignment")) 
    {
        if (generalSettings.value("print_assignment").toString().toLower()=="true")
        {
            printAssignment=true;
        }
    }

    // Read project specific settings
    if ((!cleanProjectID.isEmpty()) &&  (RTI->settingsJson.contains(cleanProjectID)))
    {
        OUT("-- Reading specific settings for project " << cleanProjectID.toStdString())
        QJsonObject projectSettings = RTI->settingsJson.value(cleanProjectID).toObject();

        if (projectSettings.contains("name")) 
        {
            projectName = projectSettings.value("name").toString();
        }
        if (projectSettings.contains("owner")) 
        {
            projectOwner = projectSettings.value("owner").toString();
        }
        if (projectSettings.contains("prefix")) 
        {
            projectPrefix = projectSettings.value("prefix").toString();
        }
        if (projectSettings.contains("preset")) 
        {
            selectedPreset=DEFAULT;
            if (projectSettings.value("preset").toString().toLower()==PRESET_NONE)
            {
                selectedPreset=NONE;

                // If the preset is set to none, then don't remove unknown tags, as
                // this would remove all tags. Also keep overlays and curves, unless
                // explicitly specified
                removeSafeTags = false;
                removeUnknownTags = false;
                removeCurves = false;
                removeOverlays = false;
            }
        }
        if (projectSettings.contains("remove_unknown_tags")) 
        {
            if (projectSettings.value("remove_unknown_tags").toString().toLower()=="false")
            {
                removeUnknownTags=false;
            }
            else
            {
                removeUnknownTags=true;
            }
        }
        if (projectSettings.contains("remove_safe_tags")) 
        {
            if (projectSettings.value("remove_safe_tags").toString().toLower()=="false")
            {
                removeSafeTags=false;
            }
            else
            {
                removeSafeTags=true;
            }
        }
        if (projectSettings.contains("remove_curves")) 
        {
            if (projectSettings.value("remove_curves").toString().toLower()=="false")
            {
                removeCurves=false;
            }
            else
            {
                removeCurves=true;
            }
        }
        if (projectSettings.contains("remove_overlays")) 
        {
            if (projectSettings.value("remove_overlays").toString().toLower()=="false")
            {
                removeOverlays=false;
            }
            else
            {
                removeOverlays=true;
            }
        }
    }
    else
    {
        OUT("-- No project-specific settings found")
    }

    // Add preset assignments    
    if (!Presets::addAssignments())
    {
        OUT("ERROR: Invalid preset settings. Aborting.")
        return false;
    }

    // Add general assignments
    foreach(const QString& key, generalSettings.keys()) 
    {
        QJsonValue value = generalSettings.value(key);
        if (!addTag(key, value.toString(), "general"))
        {
            OUT("ERROR: Invalid settings found. Aborting.")
            return false;
        }
        //qDebug() << "Key = " << key << ", Value = " << value.toString();
    }

    // Add project specific assignments
    if ((!cleanProjectID.isEmpty()) &&  (RTI->settingsJson.contains(cleanProjectID)))
    {
        QJsonObject projectAssignments = RTI->settingsJson.value(cleanProjectID).toObject();

        foreach(const QString& key, projectAssignments.keys()) 
        {
            QJsonValue value = projectAssignments.value(key);
            if (!addTag(key, value.toString(), "project"))
            {
                OUT("ERROR: Invalid assignment found. Aborting.")
                return false;
            }
            //qDebug() << "Key = " << key << ", Value = " << value.toString();
        }        
    }

    calculateHashACC();
    replaceAllParameterMacros();

    if (removeSafeTags)
    {
        OUT("-- Removing tags flagged as SAFE")

        QMapIterator<QString, TagEntry> i(tags);
        while (i.hasNext()) 
        {
            i.next();
            if (i.value().command==TagEntry::SAFE)
            {
                tags[i.key()].command = TagEntry::REMOVE;
            }
        }
    }

    if (removeUnknownTags)
    {
        OUT("-- Removing all unknown tags")
    }
    if (removeCurves)
    {
        OUT("-- Removing embedded curves")
    }
    if (removeOverlays)
    {
        OUT("-- Removing embedded overlays")
    }

    if (printAssignment)
    {
        printTags();
    }
    
    isPrepared=true;
    return true;
}


bool ModuleSettings::addTag(QString key, QString value, QString source)
{
    Uint16 group = 0;
    Uint16 element = 0;
    QString keyID = key;

    if ((!keyID.startsWith("(")) || (!keyID.endsWith(")")))
    {
        // Entry is not a tag assignment. Ignore
        return true;
    }

    keyID.chop(1);
    keyID.remove(0,1);
    QStringList items = keyID.split(",");

    if (items.length()<2)
    {
        OUT("ERROR: Invalid key format " << key.toStdString())
        return false;
    }

    bool okGroup=false;
    group = items.at(0).toUInt(&okGroup, 16);
    bool okElement=false;
    element = items.at(1).toUInt(&okElement, 16);

    if ((!okGroup) || (!okElement))
    {
        OUT("ERROR: Invalid key format " << key.toStdString())
        return false;
    }

    // Interpret the command and safe the parameter if needed
    TagEntry::Command command = TagEntry::KEEP;
    QString parameter = "";

    if (value.startsWith(COMMAND_ID_KEEP))
    {
        command = TagEntry::KEEP;
    }
    else if (value.startsWith(COMMAND_ID_SAFE))
    {
        command = TagEntry::SAFE;
    }
    else if (value.startsWith(COMMAND_ID_REMOVE))
    {
        command = TagEntry::REMOVE;
    }
    else if (value.startsWith(COMMAND_ID_CLEAR))
    {
        command = TagEntry::CLEAR;
    }
    else if (value.startsWith(QString(COMMAND_ID_SET)+"("))
    {
        command = TagEntry::SET;
        if (!value.endsWith(")"))
        {
            OUT("ERROR: Invalid command found " << value.toStdString())
            return false;
        }
        parameter = value;
        parameter.chop(1);
        parameter.remove(0,parameter.indexOf("(")+1);
    }
    else if (value.startsWith(COMMAND_ID_TRUNCDATE))
    {
        command = TagEntry::TRUNCDATE;
    }    
    else
    {
        OUT("ERROR: Invalid command found " << value.toStdString())
        return false;
    }

    // Overwrite the tag assignment if already existing, otherwise create new entry
    if (tags.contains(key))
    {
        tags[key].set(group, element, command, parameter, source);
    }
    else
    {
        tags.insert(key, TagEntry(group, element, command, parameter, source));
    }

    return true;
}


void ModuleSettings::replaceAllParameterMacros()
{
    QMapIterator<QString, TagEntry> i(tags);
    while (i.hasNext()) 
    {
        i.next();       
        if (!i.value().parameter.isEmpty())
        {
            tags[i.key()].replaceParameterMacros();
        }       
    }
}


void ModuleSettings::printTags()
{
    OUT("")
    OUT("-- TAG ASSIGNMENT --------------------------------------------------------------------")
    QMapIterator<QString, TagEntry> i(tags);
    while (i.hasNext()) 
    {
        i.next();
        // Print all assignments (except tags marked as SAFE, which would make the list hard to read)
        if (i.value().command != TagEntry::SAFE)
        {
            OUT("   " << i.key().toStdString() << ": CMD=" << i.value().getCommandName().toStdString() << ", PARAM=\"" << i.value().parameter.toStdString() << "\", SOURCE=" << i.value().source.toStdString()  << ", GROUP=" << i.value().group << ", ELEMENT=" << i.value().element)
        }
    }
    OUT("--------------------------------------------------------------------------------------")
    OUT("")
}
