#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "dcmtk/dcmdata/dcpath.h"
#include "dcmtk/dcmdata/dcerror.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/dcmdata/dcspchrs.h"
#include "dcmtk/dcmdata/dctypes.h"

#include "global.h"
#include "settings.h"


bool readSettings()
{
    if (!inputDir.exists(TASKFILE))
    {
        OUT("ERROR: Task file not found")
        return false;
    }

    QFile settingsFile(inputDir.filePath(TASKFILE));
    if (!settingsFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        OUT("ERROR: Unable to open task file")
        return false;
    }
    QByteArray data = settingsFile.readAll();
    settingsFile.close();    

    QJsonParseError errorPtr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &errorPtr);
    if (doc.isNull()) 
    {
        OUT("ERROR: Unable to parse task file")
        return false;
    }
    QJsonObject root = doc.object();
    if (!root.contains("process"))
    {
        OUT("ERROR: Invalid task file")
        return false;
    }

    QJsonObject process = root.value("process").toObject();
    if ((process.contains("retain_input_images")) && (process.value("retain_input_images").toString()=="True"))
    {
        OUT("ERROR: Rule has been configured to retain original images, which is not allowed for the anonymizer module")
        return false;
    }

    if (!process.contains("settings"))
    {
        OUT("ERROR: Invalid task file (settings missing)")
        return false;
    }

    settingsJson = process.value("settings").toObject();

    if (settingsJson.value("general").toObject().value("mode").toString()=="whitelist")
    {
        settings.mode=SETTINGS_MODE_WHITELIST;
        OUT("Using whitelist mode")
    }
    else
    {
        settings.mode=SETTINGS_MODE_BLACKLIST;
        OUT("Using blacklist mode")
    }

    if (settings.mode==SETTINGS_MODE_WHITELIST)
    {
        OUT("ERROR: The whitelist mode has not been implemented yet")
        return false;
    }

    return true;
}


bool processFile(QFileInfo currentFile)
{
    //OUT("  File " + currentFile.fileName().toStdString())

    OFString inputFilename = OFString(currentFile.absoluteFilePath().toUtf8().constData());
    OFString outputFilename = OFString(outputDir.filePath(currentFile.completeBaseName() + "_mod.dcm").toUtf8().constData());
    DcmFileFormat dcmFile;
    OFCondition readStatus = dcmFile.loadFile(inputFilename);

    if (!readStatus.good())
    {
        OUT("Unable to read DICOM file " << currentFile.fileName().toStdString());
        return false;
    }

    if (!settings.isPrepared)
    {
        QString projectName = "";
        
        // TODO: Get project name from retriving AET

        settings.prepareSettings(projectName);
    }

    OFString buffer = "";
    if ((dcmFile.getDataset()->tagExistsWithValue(DCM_StudyDescription)) && (!dcmFile.getDataset()->findAndGetOFString(DCM_StudyDescription, buffer).good())) 
    {
        OUT("Unable to read tag from file " << currentFile.fileName().toStdString());                                                                       
        return false;
    }    

    buffer = "YOYOYO";
    dcmFile.getDataset()->putAndInsertString(DCM_StudyDescription, buffer.c_str());

    OFCondition writeStatus = dcmFile.saveFile(outputFilename);

    return true;
}


bool processFiles()
{
    QStringList nameFilter;
    nameFilter << "*.dcm";
    inputFiles=inputDir.entryInfoList(nameFilter, QDir::NoDotAndDotDot | QDir::Files, QDir::Name);

    QString currentSeries = "";

    for (int i = 0; i < inputFiles.size(); ++i) 
    {
        QString seriesUID = inputFiles.at(i).fileName().split(SEPARATOR)[0];
        if (seriesUID != currentSeries)
        {
            currentSeries = seriesUID;
            OUT("Processing series " << currentSeries.toStdString());
        }
        if (!processFile(inputFiles.at(i)))
        {
            OUT("ERROR: Unable to process file " << inputFiles.at(i).fileName().toStdString())
            OUT("Aborting")
            return false;
        }
    }

    return true;
}


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        OUT("")
        OUT("mercure-anonymizer ver " << VERSION)
        OUT("--------------------------")
        OUT("")
        OUT("Usage: [input folder] [output folder]")
        OUT("")
        return 1;
    }

    inputFolder = QString(argv[1]);
    outputFolder = QString(argv[2]);

    inputDir.setPath(inputFolder);
    outputDir.setPath(outputFolder);

    if (!inputDir.exists() || !outputDir.exists())
    {
        OUT("ERROR: Input or output folder does not exist")
        return 1;
    }

    if (!readSettings()) 
    {
        OUT("ERROR: Unable to read module settings")
        return 1;
    }

    if (!processFiles())
    {
        OUT("ERROR: Unable to process files")
        return 1;
    }

    return 0;
}
