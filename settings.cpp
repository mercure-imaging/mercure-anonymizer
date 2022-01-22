#include "global.h"
#include "settings.h"
#include "runtime.h"


bool ModuleSettings::prepareSettings(QString projectID)
{
    OUT("Receiving AET is " << projectID.toStdString())

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
    dateString = QDateTime::currentDateTime().toString("MMddyyyy");
    skipDefaultAssignment=false;

    // Read general settings
    if (generalSettings.contains("name")) 
    {
        projectName = generalSettings.value("name").toString();
    }
    if (generalSettings.contains("owner")) 
    {
        projectOwner = generalSettings.value("owner").toString();
    }
    if (generalSettings.contains("skip_default_assignment")) 
    {
        if (generalSettings.value("skip_default_assignment").toString().toLower()=="true")
        {
            skipDefaultAssignment=true;
        }
    }

    // Add default assignments    
    if (!skipDefaultAssignment)
    {
        addTag("(0010,1040)", "remove", "default");
        // TODO
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
        qDebug() << "Key = " << key << ", Value = " << value.toString();
    }

    // Read project specific settings
    if ((!cleanProjectID.isEmpty()) &&  (RTI->settingsJson.contains(cleanProjectID)))
    {
        OUT("Reading specific settings for project " << cleanProjectID.toStdString())
        QJsonObject projectSettings = RTI->settingsJson.value(cleanProjectID).toObject();

        if (projectSettings.contains("name")) 
        {
            projectName = projectSettings.value("name").toString();
        }
        if (projectSettings.contains("owner")) 
        {
            projectOwner = projectSettings.value("owner").toString();
        }

        foreach(const QString& key, projectSettings.keys()) 
        {
            QJsonValue value = projectSettings.value(key);
            if (!addTag(key, value.toString(), "project"))
            {
                OUT("ERROR: Invalid settings found. Aborting.")
                return false;
            }
            qDebug() << "Key = " << key << ", Value = " << value.toString();
        }        
    }
    else
    {
        OUT("No project-specific settings found.")
    }

    replaceAllParameterMacros();

    printTags();
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

    TagEntry::Command command = TagEntry::KEEP;
    QString parameter = "";

    if (value.startsWith(COMMAND_ID_KEEP))
    {
        command = TagEntry::KEEP;
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
    else
    {
        OUT("ERROR: Invalid command found " << value.toStdString())
        return false;
    }

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
    while (i.hasNext()) {
        i.next();       
        if (!i.value().parameter.isEmpty())
        {
            tags[i.key()].replaceParameterMacros();
        }       
    }
}


void ModuleSettings::printTags()
{
    OUT("-- TAG ASSIGNMENT --------------------------------------------------------------------")
    QMapIterator<QString, TagEntry> i(tags);
    while (i.hasNext()) {
        i.next();
        OUT(i.key().toStdString() << ": GROUP=" << i.value().group << " ELEMENT=" << i.value().element << " CMD=" << i.value().getCommandName().toStdString() << " PARAM=" << i.value().parameter.toStdString() << " SOURCE=" << i.value().source.toStdString())
    }
    OUT("--------------------------------------------------------------------------------------")
}


void TagEntry::replaceParameterMacros()
{
    replaceMacro(SET_MACRO_NAME, RTI->settings.projectName);
    replaceMacro(SET_MACRO_OWNER, RTI->settings.projectOwner);
    replaceMacro(SET_MACRO_DATE, RTI->settings.dateString);
}


void TagEntry::replaceMacro(QString macro, QString value)
{
    while (parameter.indexOf(macro)>=0)
    {
        parameter.replace(parameter.indexOf(macro), macro.size(), value);
    }
}

