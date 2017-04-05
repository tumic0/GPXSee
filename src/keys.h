#ifndef KEYS_H
#define KEYS_H

#include <Qt>
#include <QKeySequence>

#define NEXT_KEY                Qt::Key_Space
#define PREV_KEY                Qt::Key_Backspace
#define FIRST_KEY               Qt::Key_Home
#define LAST_KEY                Qt::Key_End
#define MODIFIER                Qt::ShiftModifier

#define ZOOM_IN                 QKeySequence::ZoomIn
#define ZOOM_OUT                QKeySequence::ZoomOut

#define QUIT_SHORTCUT           QKeySequence(QKeySequence::Quit)
#define OPEN_SHORTCUT           QKeySequence(QKeySequence::Open)
#define CLOSE_SHORTCUT          QKeySequence(QKeySequence::Close)
#define RELOAD_SHORTCUT         QKeySequence(QKeySequence::Refresh)
#define EXPORT_SHORTCUT         QKeySequence(Qt::CTRL + Qt::Key_E)
#define SHOW_POI_SHORTCUT       QKeySequence(Qt::CTRL + Qt::Key_P)
#define SHOW_MAP_SHORTCUT       QKeySequence(Qt::CTRL + Qt::Key_M)
#define NEXT_MAP_SHORTCUT       QKeySequence(QKeySequence::Forward)
#define PREV_MAP_SHORTCUT       QKeySequence(QKeySequence::Back)
#define SHOW_GRAPHS_SHORTCUT    QKeySequence(Qt::CTRL + Qt::Key_G)
#define DISTANCE_GRAPH_SHORTCUT QKeySequence(Qt::CTRL + Qt::Key_D)
#define TIME_GRAPH_SHORTCUT     QKeySequence(Qt::CTRL + Qt::Key_T)

#ifdef Q_OS_MAC
#define FULLSCREEN_SHORTCUT     QKeySequence(Qt::META + Qt::CTRL + Qt::Key_F)
#else // Q_OS_MAC
#define FULLSCREEN_SHORTCUT     QKeySequence(Qt::Key_F11)
#endif // Q_OS_MAC

#endif // KEYS_H
