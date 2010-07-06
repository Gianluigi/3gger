# -------------------------------------------------
# Project created by QtCreator 2010-02-15T23:28:09
# -------------------------------------------------
QT += xml \
    xmlpatterns \
    testlib
TARGET = 3gger
TEMPLATE = app
SOURCES += main.cpp \
    etriggermain.cpp \
    triggermodel.cpp \
    checkboxdelegate.cpp
HEADERS += etriggermain.h \
    triggermodel.h \
    checkboxdelegate.h \
    3gr_config.h
FORMS += etriggermain.ui
RESOURCES += resources.qrc
CONFIG += debug_and_release
INCLUDEPATH  += ./qextserialport/src
QMAKE_LIBDIR += ./qextserialport/src/build

CONFIG(debug, debug|release):LIBS  += -lqextserialportd
else:LIBS  += -lqextserialport
win32:LIBS += -lsetupapi -lqextserialport1
