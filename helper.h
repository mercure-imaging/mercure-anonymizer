#ifndef MA_HELPER_H
#define MA_HELPER_H

#include <QtCore>

class Helper 
{
public:
    static QString getAETfromTagsFile(QFileInfo currentFile);
    static QString getACCfromTagsFile(QFileInfo currentFile);
    static QString getFakeName();
    static QString getFakeMRN();
    static QString getFakeACC();

    static void generateStudyUID();
    static void generateSeriesUID();
    static void generateInstanceUID();
    static void generateRandomUID();

};

#endif
