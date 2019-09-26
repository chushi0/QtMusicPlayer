#-------------------------------------------------
#
# Project created by QtCreator 2019-08-04T09:44:46
#
#-------------------------------------------------

# 版本信息
RC_FILE += versions.rc

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 音乐播放组件
QT += multimedia
# Windows 其他组件
QT += winextras

TARGET = MusicPlayer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        lyricwindow.cpp \
        main.cpp \
        listwindow.cpp \
        utils.cpp

HEADERS += \
        listwindow.h \
        lyricwindow.h \
	pch.h

FORMS += \
        listwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../Windows Dlls/MusicLyric/1.0/dll/' -lMusicLyric
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../Windows Dlls/MusicLyric/1.0/dll/' -lMusicLyricd

INCLUDEPATH += $$PWD/'../../Windows Dlls/MusicLyric/1.0/include'
DEPENDPATH += $$PWD/'../../Windows Dlls/MusicLyric/1.0/include'

win32: LIBS += -L$$PWD/'../../Windows Dlls/LyricProcessLabel/dll/' -lLyricProcessLabel

INCLUDEPATH += $$PWD/'../../Windows Dlls/LyricProcessLabel/include'
DEPENDPATH += $$PWD/'../../Windows Dlls/LyricProcessLabel/include'
