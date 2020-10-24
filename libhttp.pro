TEMPLATE = subdirs

SUBDIRS += \
  LibHttp \
  RestServer \
  Test

!build_pass:message(LibHTTP project dir: $${PWD})

#INCLUDEPATH += /usr/include/c++/8
unix:!macx: LIBS += -lstdc++

# Needed to ensure that things are built right, which you have to do yourself :(
CONFIG += ordered
CONFIG += c++17 -Wall
QMAKE_CXXFLAGS += -std=c++17

RestServer.depends = LibHttp
Test.depends = LibHttp
