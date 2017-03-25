# This file sets qmake variables for linkage against libmodbus library.
# Note: common.pri must be included before this file.

DEFINES += DLLBUILD
INCLUDEPATH += $$PWD/../libmodbus-3.1.2/src
DEPENDPATH += $$PWD/../libmodbus-3.1.2/src
win32:LIBS += -L$$PWD/../libmodbus-3.1.2/src/.libs -llibmodbus-5
unix:LIBS += -L$$PWD/../libmodbus-3.1.2/src/.libs -lmodbus