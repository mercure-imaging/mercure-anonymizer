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

    // Convert the AET to lower case to avoid problems with capitalization
    return root.value("ReceiverAET").toString().toLower();
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


#define GREEK_ALPHABET_COUNT 24
static QString greek_alphabet[]
{
    "Alpha",
    "Beta",
    "Gamma",
    "Delta",
    "Epsilon",
    "Zeta",
    "Eta",
    "Theta",
    "Iota",
    "Kappa",
    "Lambda",
    "Mu",
    "Nu",
    "Xi",
    "Omicron",
    "Pi",
    "Rho",
    "Sigma",
    "Tau",
    "Upsilon",
    "Phi",
    "Chi",
    "Psi",
    "Omega"
};

QString Helper::getFakeName()
{
    int firstIndex = QRandomGenerator::global()->bounded(0, GREEK_ALPHABET_COUNT);
    int lastIndex = QRandomGenerator::global()->bounded(0, GREEK_ALPHABET_COUNT);
    return QString("^" + greek_alphabet[lastIndex] + "^" + greek_alphabet[firstIndex] + "^^^");
}


QString Helper::getFakeMRN()
{
    // Generate a integer number from the time, ensuring that the number always increases
    QDateTime refDate(QDate(2020, 1, 1), QTime(0, 0, 0));
    return QString::number(QDateTime::currentMSecsSinceEpoch()-refDate.toMSecsSinceEpoch());
}


QString Helper::getFakeACC()
{
    // Generate a integer number from the time, ensuring that the number always increases
    // A different offset is used for the ACC and MRN to get two different numbers
    QDateTime refDate(QDate(2013, 4, 12), QTime(11, 13, 33));
    return QString::number(QDateTime::currentMSecsSinceEpoch()-refDate.toMSecsSinceEpoch());
}
