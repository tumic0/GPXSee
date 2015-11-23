#ifndef ICONS_H
#define ICONS_H

#include <QtGlobal>

#define APP_ICON         ":/icons/gpxsee.png"
#define OPEN_FILE_ICON   ":/icons/document-open.png"
#define SAVE_FILE_ICON   ":/icons/document-save.png"
#define SAVE_AS_ICON     ":/icons/document-save-as.png"
#define CLOSE_FILE_ICON  ":/icons/dialog-close.png"
#define SHOW_POI_ICON    ":/icons/flag.png"
#define SHOW_MAP_ICON    ":/icons/applications-internet.png"
#define QUIT_ICON        ":/icons/application-exit.png"
#define RELOAD_FILE_ICON ":/icons/view-refresh.png"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#define QT_ICON          ":/trolltech/qmessagebox/images/qtlogo-64.png"
#else
#define QT_ICON          ":/qt-project.org/qmessagebox/images/qtlogo-64.png"
#endif

#endif /* ICONS_H */
