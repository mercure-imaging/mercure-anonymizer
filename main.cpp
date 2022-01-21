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
#include "runtime.h"
#include "helper.h"


/*
OFString projectBuffer = "";
if ((dcmFile.getDataset()->tagExistsWithValue(DCM_StudyDescription)) && (!dcmFile.getDataset()->findAndGetOFString(DCM_RetrieveAETitle, projectBuffer).good()))
{
    OUT("Unable to read RetrieveAETitle from file " << currentFile.fileName().toStdString());
    OUT("Unable to determine target project. Aborting")
    return false;
}
projectName = QString(projectBuffer.c_str());
if (!projectName.startsWith(AET_PREFIX, Qt::CaseSensitive))
{
    OUT("Invalid format of RetrieveAETitle in file " << currentFile.fileName().toStdString());
    return false;
}
*/


bool readSettings()
{
    if (!RTI->inputDir.exists(TASKFILE))
    {
        OUT("ERROR: Task file not found")
        return false;
    }

    QFile settingsFile(RTI->inputDir.filePath(TASKFILE));
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

    RTI->settingsJson = process.value("settings").toObject();

    if (RTI->settingsJson.value("general").toObject().value("mode").toString()=="whitelist")
    {
        RTI->settings.mode=SETTINGS_MODE_WHITELIST;
        OUT("Using whitelist mode")
    }
    else
    {
        RTI->settings.mode=SETTINGS_MODE_BLACKLIST;
        OUT("Using blacklist mode")
    }

    if (RTI->settings.mode==SETTINGS_MODE_WHITELIST)
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
    OFString outputFilename = OFString(RTI->outputDir.filePath(currentFile.completeBaseName() + "_mod.dcm").toUtf8().constData());
    DcmFileFormat dcmFile;
    OFCondition readStatus = dcmFile.loadFile(inputFilename);

    if (!readStatus.good())
    {
        OUT("Unable to read DICOM file " << currentFile.fileName().toStdString());
        return false;
    }

    if (!RTI->settings.isPrepared)
    {
        QString projectName = "";

        // Get project name from retriving AET
        projectName = Helper::getAETfromTagsFile(currentFile);
        if (projectName.isEmpty())
        {
            OUT("Unable to read ReceivingAET from tags file for " << currentFile.fileName().toStdString());
            OUT("Unable to determine target project. Aborting")
            return false;
        }     

        // Compose the processing settings based on the selected project name
        RTI->settings.prepareSettings(projectName);
    }

    for (int i=0; i<RTI->settings.tags.length(); i++)
    {
        /*
        OFString buffer = "";
        if ((dcmFile.getDataset()->tagExistsWithValue(DCM_StudyDescription)) && (!dcmFile.getDataset()->findAndGetOFString(DCM_StudyDescription, buffer).good())) 
        {
            OUT("Unable to read tag from file " << currentFile.fileName().toStdString());                                                                       
            return false;
        }    
        buffer = "YOYOYO";
        dcmFile.getDataset()->putAndInsertString(DCM_StudyDescription, buffer.c_str());
        */
    }

    OFCondition writeStatus = dcmFile.saveFile(outputFilename);
    // TODO: Error logging

    return true;
}


bool processFiles()
{
    QStringList nameFilter;
    nameFilter << "*.dcm";
    RTI->inputFiles=RTI->inputDir.entryInfoList(nameFilter, QDir::NoDotAndDotDot | QDir::Files, QDir::Name);

    QString currentSeries = "";

    for (int i = 0; i < RTI->inputFiles.size(); ++i) 
    {
        QString seriesUID = RTI->inputFiles.at(i).fileName().split(SEPARATOR)[0];
        if (seriesUID != currentSeries)
        {
            currentSeries = seriesUID;
            OUT("Processing series " << currentSeries.toStdString());
        }
        if (!processFile(RTI->inputFiles.at(i)))
        {
            OUT("ERROR: Unable to process file " << RTI->inputFiles.at(i).fileName().toStdString())
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

    RTI->inputFolder = QString(argv[1]);
    RTI->outputFolder = QString(argv[2]);

    RTI->inputDir.setPath(RTI->inputFolder);
    RTI->outputDir.setPath(RTI->outputFolder);

    if (!RTI->inputDir.exists() || !RTI->outputDir.exists())
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
