TEMPLATE = subdirs

SUBDIRS += \
  RestServer \
  LibHttp \

!build_pass:message(LibHTTP project dir: $${PWD})

INCLUDEPATH += /usr/include/c++/8
unix:!macx: LIBS += -lstdc++

CONFIG += c++17 -Wall
QMAKE_CXXFLAGS += -std=c++17
