#ifndef KEYS_H
#define KEYS_H

#include <Qt>
#include <QKeySequence>

#define NEXT_KEY              Qt::Key_Space
#define PREV_KEY              Qt::Key_Backspace
#define FIRST_KEY             Qt::Key_Home
#define LAST_KEY              Qt::Key_End
#define MODIFIER              Qt::ShiftModifier

#define QUIT_SHORTCUT         QKeySequence::Quit
#define OPEN_SHORTCUT         QKeySequence::Open
#define SAVE_SHORTCUT         QKeySequence::Save
#define SAVE_AS_SHORTCUT      QKeySequence::SaveAs
#define CLOSE_SHORTCUT        QKeySequence::Close
#define RELOAD_SHORTCUT       QKeySequence::Refresh
#define SHOW_POI_SHORTCUT     QKeySequence(Qt::CTRL + Qt::Key_P)
#define SHOW_MAP_SHORTCUT     QKeySequence(Qt::CTRL + Qt::Key_M)
#define NEXT_MAP_SHORTCUT     QKeySequence::Forward
#define PREV_MAP_SHORTCUT     QKeySequence::Back
#define SHOW_GRAPHS_SHORTCUT  QKeySequence(Qt::CTRL + Qt::Key_G)
#define FULLSCREEN_SHORTCUT   QKeySequence(Qt::Key_F11)

#endif // KEYS_H
