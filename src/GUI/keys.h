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
#define PDF_EXPORT_SHORTCUT     QKeySequence(Qt::CTRL + Qt::Key_E)
#define PNG_EXPORT_SHORTCUT     QKeySequence(Qt::CTRL + Qt::Key_X)
#define SHOW_POI_SHORTCUT       QKeySequence(Qt::CTRL + Qt::Key_P)
#define SHOW_MAP_SHORTCUT       QKeySequence(Qt::CTRL + Qt::Key_M)
#define NEXT_MAP_SHORTCUT       QKeySequence(QKeySequence::Forward)
#define PREV_MAP_SHORTCUT       QKeySequence(QKeySequence::Back)
#define SHOW_GRAPHS_SHORTCUT    QKeySequence(Qt::CTRL + Qt::Key_G)
#define STATISTICS_SHORTCUT     QKeySequence(Qt::CTRL + Qt::Key_S)
#define FULLSCREEN_SHORTCUT     (QKeySequence(QKeySequence::FullScreen).isEmpty() \
                                  ? QKeySequence(Qt::Key_F11) \
                                  : QKeySequence(QKeySequence::FullScreen))

#endif // KEYS_H
