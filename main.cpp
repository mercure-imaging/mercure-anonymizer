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


bool removeUnknownTags(DcmDataset* dataset, DcmMetaInfo* /*metainfo*/)
{
    DcmStack stack;
    DcmObject* dcmObject = nullptr;
    OFCondition status = dataset->nextObject(stack, OFTrue);

    Uint16 gTag = 0;
    Uint16 eTag = 0;

    QMap<QString, TagEntry> removalList;
    QString elementID = "";

    while (status.good())
    {
        dcmObject = stack.top();
        gTag = dcmObject->getGTag();
        eTag = dcmObject->getETag();
        elementID = "(" + QStringLiteral("%1").arg(gTag, 4, 16, QLatin1Char('0')) + "," + QStringLiteral("%1").arg(eTag, 4, 16, QLatin1Char('0')) + ")";

        if (!RTI->settings.tags.contains(elementID))
        {
            removalList.insert(elementID, TagEntry(gTag, eTag, TagEntry::REMOVE, "", ""));
        }

        status = dataset->nextObject(stack, OFTrue);
    }

    QMapIterator<QString, TagEntry> i(removalList);
    while (i.hasNext()) 
    {
        i.next();
        DcmTagKey tagKey(i.value().group, i.value().element);
        if ((dataset->tagExists(tagKey)) && (dataset->findAndDeleteElement(tagKey).bad()))
        {
            OUT("ERROR: Unable to REMOVE tag " << i.key().toStdString())
            return false;
        }
    }

    return true;
}


bool removeCurvesOverlays(DcmDataset* dataset, DcmMetaInfo* /*metainfo*/)
{
    if ((!RTI->settings.removeCurves) && (!RTI->settings.removeOverlays))
    {
        // Nothing to be done
        return true;
    }

    DcmStack stack;
    DcmObject* dcmObject = nullptr;
    OFCondition status = dataset->nextObject(stack, OFTrue);

    Uint16 gTag = 0;
    Uint16 eTag = 0;

    QMap<QString, TagEntry> removalList;
    QString elementID = "";

    bool removeTag = false;
    while (status.good())
    {
        dcmObject = stack.top();
        gTag = dcmObject->getGTag();

        removeTag = false;
        if ((RTI->settings.removeCurves) && (gTag >= 0x5000) && (gTag <= 0x501E))
        {
            removeTag = true;
        }
        if ((RTI->settings.removeOverlays) && (gTag >= 0x6000) && (gTag <= 0x601E))
        {
            removeTag = true;
        }        
        if (removeTag)
        {
            elementID = "(" + QStringLiteral("%1").arg(gTag, 4, 16, QLatin1Char('0')) + "," + QStringLiteral("%1").arg(eTag, 4, 16, QLatin1Char('0')) + ")";
            removalList.insert(elementID, TagEntry(gTag, eTag, TagEntry::REMOVE, "", ""));
        }

        status = dataset->nextObject(stack, OFTrue);
    }

    QMapIterator<QString, TagEntry> i(removalList);
    while (i.hasNext()) 
    {
        i.next();
        DcmTagKey tagKey(i.value().group, i.value().element);
        if ((dataset->tagExists(tagKey)) && (dataset->findAndDeleteElement(tagKey).bad()))
        {
            OUT("ERROR: Unable to REMOVE tag " << i.key().toStdString())
            return false;
        }
    }

    return true;
}


bool processTags(DcmDataset* dataset, DcmMetaInfo* metainfo)
{   
    QString currentValue = "";
    QMapIterator<QString, TagEntry> i(RTI->settings.tags);

    // Create deep copy of the values prior to processing, so that values can be 
    // cross-referenced via the SET command
    DcmDataset originalValues;
    originalValues.copyFrom(*dataset);

    while (i.hasNext()) 
    {
        i.next();
        DcmTagKey tagKey(i.value().group, i.value().element);

        switch (i.value().command)
        {
        case TagEntry::KEEP:
        case TagEntry::SAFE:
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
        case TagEntry::TRUNCDATE:
            currentValue = "";
            if (dataset->tagExists(tagKey))
            {
                OFString buffer = "";
                if (dataset->findAndGetOFString(tagKey, buffer).bad())
                {
                    OUT("ERROR: Unable to read value from tag " << i.key().toStdString())
                    return false;
                }
                currentValue = QString(buffer.c_str());
            }
            if (currentValue.length() == 8) 
            {
                currentValue = currentValue.left(4)+"0101";
            }
            else 
            {
                currentValue = "";
            }
            if (dataset->putAndInsertString(tagKey, currentValue.toUtf8(), OFTrue).bad())
            {
                OUT("ERROR: Unable to insert TRUNCDATE tag " << i.key().toStdString())
                return false;
            }
            break;
        case TagEntry::SET:
            QString setParameter = i.value().parameter;

            // If the parameter for the tag contains the value macro @value@, fetch the current value and replace the macro
            if (setParameter.contains(SET_MACRO_VALUE))
            {
                currentValue = "";
                QString valueMacro = SET_MACRO_VALUE;
                if (dataset->tagExists(tagKey))
                {
                    OFString buffer = "";
                    if (dataset->findAndGetOFString(tagKey, buffer).bad())
                    {
                        OUT("ERROR: Unable to read value from tag " << i.key().toStdString())        
                        return false;
                    }
                    currentValue = QString(buffer.c_str());
                }
                // Replace all occurrances of the macro in the parameter string with the value
                while (setParameter.indexOf(valueMacro)>=0)
                {
                    setParameter.replace(setParameter.indexOf(valueMacro), valueMacro.size(), currentValue);
                }                
            }

            // If the parameter for the tag contains the value macro referencing another tag @value(xxxx,xxxx)@
            while (setParameter.indexOf(SET_MACRO_OTHERVALUE_OPEN)>=0)
            {
                int closePosition = setParameter.indexOf(SET_MACRO_OTHERVALUE_CLOSE);
                if (closePosition < 0) 
                {
                    OUT("ERROR: Invalid format of SET parameter for " << i.key().toStdString())        
                    return false;
                }
                int startPosition = setParameter.indexOf(SET_MACRO_OTHERVALUE_OPEN);
                int paramStart = startPosition + QString(SET_MACRO_OTHERVALUE_OPEN).size();

                QString fullMacro = setParameter.mid(startPosition, closePosition-startPosition+QString(SET_MACRO_OTHERVALUE_CLOSE).size());
                QString macroParameter = setParameter.mid(paramStart, closePosition-paramStart);
                if ((macroParameter.size()!=9) || (macroParameter[4] != ",")) 
                {
                    OUT("ERROR: Invalid format of SET parameter for " << i.key().toStdString() << ": " << fullMacro.toStdString())        
                    return false;
                }

                bool okGroup=false;
                bool okElement=false;
                Uint16 group = macroParameter.mid(0,4).toUInt(&okGroup, 16);
                Uint16 element = macroParameter.mid(5,4).toUInt(&okElement, 16);
                if ((!okGroup) || (!okElement))
                {
                    OUT("ERROR: Invalid format of SET parameter for " << i.key().toStdString() << ": " << fullMacro.toStdString())        
                    return false;
                }

                QString replacementValue = "";                
                DcmTagKey refKey(group, element);
                if (originalValues.tagExists(refKey))
                {
                    OFString refBuffer = "";
                    if (originalValues.findAndGetOFString(refKey, refBuffer).bad())
                    {
                        OUT("ERROR: Unable to read value for macro " << fullMacro.toStdString())        
                        return false;
                    }
                    replacementValue = QString(refBuffer.c_str());
                }
                
                setParameter.replace(setParameter.indexOf(fullMacro), fullMacro.size(), replacementValue);
            }                                

            if (dataset->putAndInsertString(tagKey, setParameter.toUtf8(), OFTrue).bad())
            {
                OUT("ERROR: Unable to SET tag " << i.key().toStdString())
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

    if (!removeCurvesOverlays(dcmFile.getDataset(), dcmFile.getMetaInfo()))
    {
        OUT("Unable to remove curves or overlays from file " << currentFile.fileName().toStdString());
        return false;
    }

    if (RTI->settings.removeUnknownTags)
    {
        if (!removeUnknownTags(dcmFile.getDataset(), dcmFile.getMetaInfo()))
        {
            OUT("Unable to remove unknown tags from file " << currentFile.fileName().toStdString());
            return false;
        }
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
    Helper::generateRandomUID();  

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
    OUT("mercure-anonymizer Version " << VERSION)
    OUT("---------------------------------------")
    OUT("")
    OUT("WARNING: This tool comes without any warranties.")
    OUT("WARNING: Operation of the software is solely at the user’s own risk.")    
    OUT("WARNING: The authors take no responsibility of any kind.")
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
    int durationSecs = static_cast<int>(startTime.secsTo(endTime));
    OUT("Done (" << durationSecs << " secs)")
    OUT("Processed files: " << RTI->processedFiles)    
    OUT("")
    return 0;
}
