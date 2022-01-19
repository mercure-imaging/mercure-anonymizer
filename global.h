#ifndef MA_GLOBAL_H
#define MA_GLOBAL_H

#include <QtCore>
#include "settings.h"

#define VERSION "0.1"
#define OUT(a) std::cout << a << std::endl;
#define TASKFILE "task.json"
#define SEPARATOR '#'

ModuleSettings settings;
QJsonObject settingsJson;

QString inputFolder = "";
QString outputFolder = "";

QDir inputDir;
QDir outputDir;

QFileInfoList inputFiles;


#endif
