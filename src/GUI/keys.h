#ifndef KEYS_H
#define KEYS_H

#include <Qt>
#include <QKeySequence>

#define NEXT_KEY                Qt::Key_Space
#define PREV_KEY                Qt::Key_Backspace
#define FIRST_KEY               Qt::Key_Home
#define LAST_KEY                Qt::Key_End
#define MODIFIER_KEY            Qt::Key_Shift
#define MODIFIER                Qt::ShiftModifier
#define ZOOM_IN                 Qt::Key_Plus
#define ZOOM_OUT                Qt::Key_Minus
#define TOGGLE_GRAPH_TYPE_KEY   Qt::Key_X
#define TOGGLE_TIME_TYPE_KEY    Qt::Key_T
#define TOGGLE_MARKER_INFO_KEY  Qt::Key_I

#define QUIT_SHORTCUT           QKeySequence(QKeySequence::Quit)
#define OPEN_SHORTCUT           QKeySequence(QKeySequence::Open)
#define CLOSE_SHORTCUT          QKeySequence(QKeySequence::Close)
#define RELOAD_SHORTCUT         QKeySequence(QKeySequence::Refresh)
#define PDF_EXPORT_SHORTCUT     QKeySequence(Qt::CTRL | Qt::Key_E)
#define PNG_EXPORT_SHORTCUT     QKeySequence(Qt::CTRL | Qt::Key_X)
#define SHOW_POI_SHORTCUT       QKeySequence(Qt::CTRL | Qt::Key_I)
#define SHOW_MAP_SHORTCUT       QKeySequence(Qt::CTRL | Qt::Key_M)
#define NEXT_MAP_SHORTCUT       QKeySequence(QKeySequence::Forward)
#define PREV_MAP_SHORTCUT       QKeySequence(QKeySequence::Back)
#define NEXT_TAB_SHORTCUT       QKeySequence(QKeySequence::NextChild)
#define PREV_TAB_SHORTCUT       QKeySequence(QKeySequence::PreviousChild)
#define SHOW_GRAPHS_SHORTCUT    QKeySequence(Qt::CTRL | Qt::Key_G)
#define STATISTICS_SHORTCUT     QKeySequence(Qt::CTRL | Qt::Key_S)
#define DOWNLOAD_DEM_SHORTCUT   QKeySequence(Qt::CTRL | Qt::Key_D)
#define SHOW_TRACKS_SHORTCUT    QKeySequence(Qt::CTRL | Qt::Key_T)
#define SHOW_ROUTES_SHORTCUT    QKeySequence(Qt::CTRL | Qt::Key_R)
#define SHOW_AREAS_SHORTCUT     QKeySequence(Qt::CTRL | Qt::Key_A)
#define SHOW_WAYPOINTS_SHORTCUT QKeySequence(Qt::CTRL | Qt::Key_P)
#define FULLSCREEN_SHORTCUT     (QKeySequence(QKeySequence::FullScreen).isEmpty() \
                                  ? QKeySequence(Qt::Key_F11) \
                                  : QKeySequence(QKeySequence::FullScreen))

#endif // KEYS_H
