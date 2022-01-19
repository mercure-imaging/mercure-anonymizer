#include <QtCore>
#include "runtime.h"
#include "settings.h"

Runtime* Runtime::pSingleton=0;


Runtime* Runtime::getInstance()
{
    if (pSingleton==0)
    {
        pSingleton=new Runtime();
    }
    return pSingleton;
}

