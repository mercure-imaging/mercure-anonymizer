#include "dcmtk/dcmdata/dcuid.h"

#include "global.h"
#include "runtime.h"
#include "settings.h"
#include "helper.h"


QString Helper::getAETfromTagsFile(QFileInfo currentFile)
{
    QString filePath=currentFile.absolutePath() + "/" + currentFile.completeBaseName() + ".tags";
    QFile tagsFile(filePath);
    if (!tagsFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        OUT("ERROR: Unable to open tags file")
        return "";
    }
    QByteArray data = tagsFile.readAll();
    tagsFile.close();    

    QJsonParseError errorPtr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &errorPtr);
    if (doc.isNull()) 
    {
        OUT("ERROR: Unable to parse tags file")
        return "";
    }
    QJsonObject root = doc.object();
    if (!root.contains("ReceiverAET"))
    {
        OUT("ERROR: Invalid tags file (ReceiverAET not found)")
        return "";
    }

    return root.value("ReceiverAET").toString();
}


void Helper::generateStudyUID()
{
    char uid[100];
    dcmGenerateUniqueIdentifier(uid, SITE_STUDY_UID_ROOT);
    RTI->newStudyUID=QString(uid);
}


void Helper::generateSeriesUID()
{
    char uid[100];
    dcmGenerateUniqueIdentifier(uid, SITE_SERIES_UID_ROOT);
    RTI->newSeriesUID=QString(uid);
}


void Helper::generateInstanceUID()
{
    char uid[100];
    dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT);
    RTI->newInstanceUID=QString(uid);
}


void Helper::generateRandomUID()
{
    char uid[100];
    dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT);
    RTI->randomUID=QString(uid);
}
