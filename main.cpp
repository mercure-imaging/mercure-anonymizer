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


bool processTags(DcmDataset* dataset, DcmMetaInfo* metainfo)
{   
    QMapIterator<QString, TagEntry> i(RTI->settings.tags);
    
    while (i.hasNext()) 
    {
        i.next();
        DcmTagKey tagKey(i.value().group, i.value().element);

        switch (i.value().command)
        {
        case TagEntry::KEEP:
            break;
        case TagEntry::REMOVE:
            if ((dataset->tagExists(tagKey)) && (dataset->findAndDeleteElement(tagKey).bad()))
            {
                OUT("ERROR: Unable to REMOVE tag " << i.key().toStdString())
                return false;
            }
            break;
        case TagEntry::CLEAR: 
            if (dataset->insertEmptyElement(tagKey, OFTrue).bad())
            {
                OUT("ERROR: Unable to CLEAR tag " << i.key().toStdString())
                return false;
            }
            break;
        case TagEntry::SET: 
            if (dataset->putAndInsertString(tagKey, i.value().parameter.toUtf8(), OFTrue).bad())
            {
                OUT("ERROR: Unable to CLEAR tag " << i.key().toStdString())
                return false;
            }
            break;
        }
    }

    metainfo->putAndInsertString(DCM_MediaStorageSOPInstanceUID, RTI->newInstanceUID.toUtf8(), OFTrue);
    dataset->putAndInsertString(DCM_SOPInstanceUID, RTI->newInstanceUID.toUtf8(), OFTrue);
    if (!RTI->newSeriesUID.isEmpty())
    {
        dataset->putAndInsertString(DCM_SeriesInstanceUID, RTI->newSeriesUID.toUtf8(), OFTrue);
    }
    if (!RTI->newStudyUID.isEmpty())
    {
        dataset->putAndInsertString(DCM_StudyInstanceUID, RTI->newStudyUID.toUtf8(), OFTrue);
    }

    return true;
}


bool processFile(QFileInfo currentFile)
{
    //OUT("  File " + currentFile.fileName().toStdString())
    
    OFString inputFilename = OFString(currentFile.absoluteFilePath().toUtf8().constData());
    OFString outputFilename = OFString(RTI->outputDir.filePath(RTI->newSeriesUID +"#" + RTI->newInstanceUID + ".dcm").toUtf8().constData());

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
        if (!RTI->settings.prepareSettings(projectName))
        {
            return false;
        }
    }

    if (!processTags(dcmFile.getDataset(), dcmFile.getMetaInfo()))
    {
        OUT("Unable to process tags of file " << currentFile.fileName().toStdString());
        return false;
    }

    OFCondition writeStatus = dcmFile.saveFile(outputFilename);
    if (!writeStatus.good())
    {
        OUT("Unable to write DICOM file " << outputFilename.c_str());
        return false;       
    }

    RTI->processedFiles++;
    return true;
}


bool processFiles()
{
    QStringList nameFilter;
    nameFilter << "*.dcm";
    RTI->inputFiles=RTI->inputDir.entryInfoList(nameFilter, QDir::NoDotAndDotDot | QDir::Files, QDir::Name);

    Helper::generateStudyUID();

    QString currentSeries = "";
    for (int i = 0; i < RTI->inputFiles.size(); ++i) 
    {        
        QString seriesUID = RTI->inputFiles.at(i).fileName().split(SEPARATOR)[0];
        if (seriesUID != currentSeries)
        {
            currentSeries = seriesUID;
            OUT("Processing series " << currentSeries.toStdString());
            Helper::generateSeriesUID();     
        }                

        Helper::generateInstanceUID();

        if (!processFile(RTI->inputFiles.at(i)))
        {
            OUT("ERROR: Unable to process file " << RTI->inputFiles.at(i).fileName().toStdString())
            OUT("ERROR: Aborting")
            return false;
        }
    }

    return true;
}


int main(int argc, char *argv[])
{
    OUT("")
    OUT("mercure-anonymizer ver " << VERSION)
    OUT("--------------------------")
    OUT("")

    if (argc < 2)
    {
        OUT("Usage: [input folder] [output folder]")
        OUT("")
        return 1;
    }

    QDateTime startTime = QDateTime::currentDateTime();

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

    QDateTime endTime = QDateTime::currentDateTime();
    int durationSecs = (int) startTime.secsTo(endTime);
    OUT("Done (" << durationSecs << " secs)")
    OUT("Processed files: " << RTI->processedFiles)    
    OUT("")
    return 0;
}
