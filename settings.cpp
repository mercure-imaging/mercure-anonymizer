#include "global.h"
#include "settings.h"
#include "runtime.h"


void ModuleSettings::prepareSettings(QString projectID)
{
    OUT("Receiving AET is " << projectID.toStdString())

    QString projectName = projectID;
    if (projectName.startsWith(AET_PREFIX))
    {
        // Chop the prefix from the front of the AET
        projectName.remove(0,QString(AET_PREFIX).length());
    }

    QJsonObject generalSettings = RTI->settingsJson.value("general").toObject();

    // Add default assignment
    tags.clear();
    RTI->settings.projectOwner = "Unknown";
    RTI->settings.projectName = "Undefined";
    RTI->settings.dateString = QDateTime::currentDateTime().toString("MMddyyyy");

    // Read general settings
    foreach(const QString& key, generalSettings.keys()) {
        QJsonValue value = generalSettings.value(key);
        qDebug() << "Key = " << key << ", Value = " << value.toString();
    }

    // Read project specific settings
    if ((!projectName.isEmpty()) &&  (RTI->settingsJson.contains(projectName)))
    {
        OUT("Reading specific settings for project " << projectName.toStdString())
        QJsonObject projectSettings = RTI->settingsJson.value(projectName).toObject();
        foreach(const QString& key, projectSettings.keys()) {
            QJsonValue value = projectSettings.value(key);
            qDebug() << "Key = " << key << ", Value = " << value.toString();
        }        
    }
    else
    {
        OUT("No project-specific settings found.")
    }

    isPrepared=true;
}


bool ModuleSettings::addTag(QString key, QString value)
{
    Uint16 group = 0;
    Uint16 element = 0;

    if ((key.startsWith("(")) && (key.endsWith(")")))
    {
        key.chop(1);
        key.remove(0,1);
        QStringList items = key.split(",");

        if (items.length()<2)
        {
            return false;
        }

        bool okGroup=false;
        group = items.at(0).toUInt(&okGroup, 16);
        bool okElement=false;
        element = items.at(1).toUInt(&okElement, 16);
    }
    else
    {
        return false;
    }

}
