include(../plugins_common.pri)

HEADERS += itemfakevim.h
SOURCES += itemfakevim.cpp
FORMS   += itemfakevimsettings.ui
TARGET   = $$qtLibraryTarget(itemfakevim)
RESOURCES += itemfakevim.qrc
DEFINES += QTCREATOR_UTILS_STATIC_LIB

include(fakevim/fakevim.pri)

