#---------------------------------------------------------------------------------------
# @功能：      获取指定文件夹路径下所有文件大小信息
# @编译器：     Desktop Qt 6.8.0 MSVC2022 64bit
# @Qt IDE：    D:/Qt/Qt6_8/Tools/QtCreator/bin/qtcreator.exe
#
# @开发者     mhf
# @邮箱       1603291350@qq.com
# @时间       2025-02-28 20:27:41
# @备注      需要先将ScanFile\ScanFileLib文件夹中工程编程生成动态库，然后ScanFile才可以编译
#           由于使用的是windows api，所以只支持windows
#---------------------------------------------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

DESTDIR = $$PWD/bin
CONFIG += c++17

SOURCES += \
    main.cpp \
    widget.cpp

HEADERS += \
    widget.h

FORMS += \
    widget.ui


INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/ -lScanFile
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/ -lScanFiled
