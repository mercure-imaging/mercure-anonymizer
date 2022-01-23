#ifndef MA_HELPER_H
#define MA_HELPER_H

#include <QtCore>

class Helper 
{
public:
    static QString getAETfromTagsFile(QFileInfo currentFile);

    static void generateStudyUID();
    static void generateSeriesUID();
    static void generateInstanceUID();
};

#endif
