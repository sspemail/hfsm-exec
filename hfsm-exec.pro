QT       += core xml xmlpatterns
QT       -= gui

TARGET = hfsm-exec

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES +=  src/main.cpp \
            src/statemachine.cpp \
            src/transition.cpp \
            src/parameter_server.cpp \
    src/api.cpp

HEADERS += inc/statemachine.h \
           inc/transition.h \
           inc/parameter_server.h \
    inc/api.h

INCLUDEPATH += inc

LIBS += -L/usr/local/lib/ -lbooster -lcppcms

INCLUDEPATH += /usr/local/include/

release: DESTDIR = build/release
debug:   DESTDIR = build/debug

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui
