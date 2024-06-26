QT += core
QT -= gui
CONFIG += console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += /usr/local/include/dcmtk/dcmnet/
INCLUDEPATH += /usr/local/include/dcmtk/config/

LIBS += -ldcmdata -loflog -lofstd
LIBS += -lz -ldl -lpthread

SOURCES += main.cpp
SOURCES += settings.cpp
SOURCES += runtime.cpp
SOURCES += helper.cpp
SOURCES += presets.cpp
SOURCES += external/siphash/siphash.c

TARGET = ./bin/mercure-anonymizer

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

docker.commands = docker build . -t mercureimaging/mercure-anonymizer:latest
QMAKE_EXTRA_TARGETS += docker
