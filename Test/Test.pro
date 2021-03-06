include(gtest_dependency.pri)
include(../qmake-target-platform.pri)
include(../qmake-destination-path.pri)

TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG += thread
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++17

# --coverage option is synonym for: -fprofile-arcs -ftest-coverage -lgcov


HEADERS += \
  Routers/http_custom_router.h \
  Suites/http_server_test.h \
  Suites/port_binding_test.h \
  consts.h \
  http_server_test.h

SOURCES += \
        Routers/http_custom_router.cpp \
        Suites/http_server_test.cpp \
        Suites/port_binding_test.cpp \
        main.cpp

INCLUDEPATH += $$PWD/../
message("Path: ["$${INCLUDEPATH}"]")

DESTDIR     = $$PWD/../binaries/$$DESTINATION_PATH
OBJECTS_DIR = $$PWD/../build/$$DESTINATION_PATH/TEST/.obj
MOC_DIR     = $$PWD/../build/$$DESTINATION_PATH/TEST/.moc
RCC_DIR     = $$PWD/../build/$$DESTINATION_PATH/TEST/.qrc
UI_DIR      = $$PWD/../build/$$DESTINATION_PATH/TEST/.ui
message(App output dir: $${DESTDIR})

CONFIG(debug) {
  QMAKE_CXXFLAGS += --coverage
  QMAKE_LFLAGS += --coverage
  QMAKE_PRE_LINK = rm -f $$OBJECTS_DIR/*gcda
  !build_pass:message(cleaning previous: rm -f "$$OBJECTS_DIR/*gcda")
}

INCLUDEPATH += $$PWD/../LibHttp
DEPENDPATH += $$PWD/../LibHttp

LIBS += -lstdc++fs
LIBS += -pthread
LIBS += -lcrypto -lssl -lboost_regex
LIBS += -L$$DESTDIR/ -lLibHttp

PRE_TARGETDEPS += $$DESTDIR/libLibHttp.a

message(Target dependencies: $${DESTDIR})
