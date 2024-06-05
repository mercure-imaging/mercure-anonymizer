#include <QtCore>
#include "runtime.h"
#include "settings.h"
#include "helper.h"


Runtime* Runtime::pSingleton=0;


Runtime* Runtime::getInstance()
{
    if (pSingleton==0)
    {
        pSingleton=new Runtime();
    }
    return pSingleton;
}


Runtime::Runtime()
{
    inputFolder = "";
    outputFolder = "";
    inputFiles.clear();

    newStudyUID = "";
    newSeriesUID = "";
    newInstanceUID = "";

    // Make sure that we are at least 1ms behind any prior anonymization to ensure we get a unique MRN/ACC
    QThread::msleep(1);
    fakeMRN = Helper::getFakeMRN(); 
    fakeACC = Helper::getFakeACC();
    fakeName = Helper::getFakeName();

    hashACC = "";
    accUsedForHash = "";

    dateString = QDateTime::currentDateTime().toString("MMddyyyy");
    timeString = QDateTime::currentDateTime().toString("hhmmsszzz");

    processedFiles=0;
}
