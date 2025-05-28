#ifndef MA_RUNTIME_H
#define MA_RUNTIME_H

#include <QtCore>
#include "settings.h"

#define RTI Runtime::getInstance()

class Runtime 
{
public: 
    Runtime();
    static Runtime* getInstance();

    ModuleSettings settings;
    QJsonObject settingsJson;

    QString inputFolder;
    QString outputFolder;
    QString persistentFolder;

    QDir inputDir;
    QDir outputDir;
    QDir persistentDir;

    QFileInfoList inputFiles;

    QString newStudyUID;
    QString newSeriesUID;
    QString newInstanceUID;
    QString randomUID;
    QString dateString;
    QString timeString;

    QString fakeMRN;
    QString fakeACC;
    QString fakeName;

    QString hashACC;
    QString accUsedForHash;

    int persistentCounter;
    bool counterMacroFound;
    bool journalEntryCreated;

    int processedFiles;

    bool isPersistentAvailable() 
    {
        return (!persistentFolder.isEmpty()); 
    }

private:
    static Runtime* pSingleton;

};


#endif
