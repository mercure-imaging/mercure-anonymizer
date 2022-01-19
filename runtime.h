#ifndef MA_RUNTIME_H
#define MA_RUNTIME_H

#include <QtCore>
#include "settings.h"

#define RTI Runtime::getInstance()

class Runtime 
{
public: 
    Runtime() 
    {
        inputFolder = "";
        outputFolder = "";
    }

    static Runtime* getInstance();


    ModuleSettings settings;
    QJsonObject settingsJson;

    QString inputFolder;
    QString outputFolder;

    QDir inputDir;
    QDir outputDir;

    QFileInfoList inputFiles;

private:
    static Runtime* pSingleton;

};


#endif

