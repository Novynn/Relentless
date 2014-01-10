TEMPLATE      = lib
CONFIG       += qt plugin debug_and_release #link_prl
CONFIG       -= embed_manifest_dll

INCLUDEPATH += ../../core/

DESTDIR       = ../../bin/plugins

HEADERS += ../../core/plugins/plugin.h
