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

    newStudyUID="";
    newSeriesUID="";
    newInstanceUID="";

    fakeName = Helper::getFakeName();
    
    processedFiles=0;
}
