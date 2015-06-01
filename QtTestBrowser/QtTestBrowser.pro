# -------------------------------------------------------------------
# Project file for the QtTestBrowser binary
#
# See 'Tools/qmake/README' for an overview of the build system
# -------------------------------------------------------------------

TEMPLATE = app

INCLUDEPATH += \
    $${ROOT_WEBKIT_DIR}/Source/WebCore/platform/qt \
    $${ROOT_WEBKIT_DIR}/Source/WebKit/qt/WebCoreSupport \
    $${ROOT_WEBKIT_DIR}/Tools/DumpRenderTree/qt/ \
    $${ROOT_WEBKIT_DIR}/Source/WTF

SOURCES += \
    locationedit.cpp \
    launcherwindow.cpp \
    qttestbrowser.cpp \
    mainwindow.cpp \
    urlloader.cpp \
    utils.cpp \
    webpage.cpp \
    webview.cpp \
    fpstimer.cpp \
    cookiejar.cpp \
    ccnxwebpage.cpp \
    ccnxnetworkaccessmanager.cpp \
    ccnxnetworkreply.cpp \
    ccnximpl.cpp \
    ccnxdownloader.cpp \
    ccnxjavascriptobject.cpp \
    xmlccnxrequest.cpp \
    ccntrietree.cpp

HEADERS += \
    locationedit.h \
    launcherwindow.h \
    mainwindow.h \
    urlloader.h \
    utils.h \
    webinspector.h \
    webpage.h \
    webview.h \
    fpstimer.h \
    cookiejar.h \
    ccnxwebpage.h \
    ccnxnetworkaccessmanager.h \
    ccnxnetworkreply.h \
    ccnximpl.h \
    ccnxdownloader.h \
    ccnxjavascriptobject.h \
    xmlccnxrequest.h \
    ccntrietree.h


WEBKIT += wtf webcore

DESTDIR = $$ROOT_BUILD_DIR/bin

QT += network webkitwidgets widgets
have?(QTPRINTSUPPORT): QT += printsupport

macx:QT += xml

linux:LIBS += -lccn -lcrypto

have?(FONTCONFIG): PKGCONFIG += fontconfig

contains(QT_CONFIG, opengl) {
    QT += opengl
    DEFINES += QT_CONFIGURED_WITH_OPENGL
}

RESOURCES += \
    QtTestBrowser.qrc
