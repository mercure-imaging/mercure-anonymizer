#include "global.h"
#include "settings.h"
#include "runtime.h"


void ModuleSettings::prepareSettings(QString projectID)
{
    tags.clear();

    // Add default assignment

    // Read general settings


    // Read project specific settings
    if (RTI->settingsJson.contains(projectID))
    {
        OUT("Reading specific settings for project " << projectID.toStdString())
        QJsonObject projectSettings = RTI->settingsJson.value(projectID).toObject();
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
