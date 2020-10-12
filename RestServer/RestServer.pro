TEMPLATE = app
CONFIG += console c++17 -Wall
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
  RestServer.cpp \
  http_custom_router.cpp

LIBS += -lstdc++fs
LIBS += -pthread
LIBS += -lcrypto -lssl -lboost_regex

#Using libstdc++ gnu 8:
INCLUDEPATH += /usr/include/c++/8
INCLUDEPATH += $$PWD/../
message("Path: ["$${INCLUDEPATH}"]")

HEADERS += \
  http_custom_router.h

DISTFILES += \
  ../websocket/echo_client.html \
  ../websocket/echo_client_ssl.html

include(../qmake-target-platform.pri)
include(../qmake-destination-path.pri)

DESTDIR     = $$PWD/../binaries/$$DESTINATION_PATH
OBJECTS_DIR = $$PWD/../build/$$DESTINATION_PATH/.obj
MOC_DIR     = $$PWD/../build/$$DESTINATION_PATH/.moc
RCC_DIR     = $$PWD/../build/$$DESTINATION_PATH/.qrc
UI_DIR      = $$PWD/../build/$$DESTINATION_PATH/.ui
message(App output dir: $${DESTDIR})
