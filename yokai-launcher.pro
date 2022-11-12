###
### yokaiLauncher
###

TEMPLATE = app
TARGET = yokaiLauncher
INCLUDEPATH += .

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#
# TODO: Try to compile with MSVC
#
# QMAKE_SPEC=linux-llvm
# INCLUDEPATH += /home/kitsune/src/mxe/usr/x86_64-w64-mingw32.static/qt5/include/QtWinExtras
# INCLUDEPATH += /home/kitsune/src/mxe/usr/x86_64-w64-mingw32.static/qt5/include/QtWidgets
# INCLUDEPATH += /home/kitsune/src/mxe/usr/x86_64-w64-mingw32.static/qt5/include/
INCLUDEPATH += include/
# QMAKE_LINK = /opt/msvc/bin/x64/cl
# QMAKE_CXX = /opt/msvc/bin/x64/cl
# QMAKE_CC = /opt/msvc/bin/x64/cl
#
# Trying with wine compiler:
# QMAKE_CXXFLAGS += -mno-cygwin
#

QT += uitools
QT += widgets
QT += svg
QT += network

HEADERS = include/launcher.h
FORMS += ui/client.ui
FORMS += ui/dcon.ui
FORMS += ui/melement.ui
FORMS += ui/dirdialog.ui
SOURCES += src/config_manager.cpp
SOURCES += src/melement.cpp
SOURCES += src/launcher.cpp
RESOURCES += res.qrc
QMAKE_CXXFLAGS += -DBUILDID=$$system(date '+%y%m%d%H%M%S') -D_VERSION="0.2"
LIBS += -lyaml-cpp

win32:CONFIG += win
win64:CONFIG += win
#win32:CONFIG += console
#win64:CONFIG += console
win:LIBS += -liconv -lcharset -ladvapi32 -lcrypt32 -lwldap32 -lzstd -lz -lws2_32 -lpthread
win:RC_ICONS += assets/0.ico
QMAKE_TARGET_COMPANY = "Vilafox (MyNameIsKitsune)"
QMAKE_TARGET_DESCRIPTION = "Simple minecraft launcher writen on C++ and Qt"
QMAKE_TARGET_COPYRIGHT = "Vilafox (https://github.com/OneGameKoTT/yokaiLauncher)"
QMAKE_TARGET_PRODUCT = "yokaiLauncher"
