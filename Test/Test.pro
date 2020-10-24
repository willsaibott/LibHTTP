include(gtest_dependency.pri)
include(../qmake-target-platform.pri)
include(../qmake-destination-path.pri)

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += thread
CONFIG -= qt

HEADERS += \
        tst_httpservertest.h

SOURCES += \
        main.cpp

INCLUDEPATH += $$PWD/../
message("Path: ["$${INCLUDEPATH}"]")

DESTDIR     = $$PWD/../binaries/$$DESTINATION_PATH
OBJECTS_DIR = $$PWD/../build/$$DESTINATION_PATH/.obj
MOC_DIR     = $$PWD/../build/$$DESTINATION_PATH/.moc
RCC_DIR     = $$PWD/../build/$$DESTINATION_PATH/.qrc
UI_DIR      = $$PWD/../build/$$DESTINATION_PATH/.ui
message(App output dir: $${DESTDIR})

INCLUDEPATH += $$PWD/../LibHttp
DEPENDPATH += $$PWD/../LibHttp

LIBS += -lstdc++fs
LIBS += -pthread
LIBS += -lcrypto -lssl -lboost_regex
LIBS += -L$$DESTDIR/ -lLibHttp

PRE_TARGETDEPS += $$DESTDIR/libLibHttp.a
message(Target dependencies: $${DESTDIR})
