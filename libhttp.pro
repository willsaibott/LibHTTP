TEMPLATE = subdirs

SUBDIRS += \
  RestServer \
  LibHttp \

!build_pass:message(LibHTTP project dir: $${PWD})

INCLUDEPATH += /usr/include/c++/8
unix:!macx: LIBS += -lstdc++
