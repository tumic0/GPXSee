#ifndef POPUP_H
#define POPUP_H

class QPoint;
class QString;
class QWidget;

class Popup
{
public:
	static void show(const QPoint &pos, const QString &text, QWidget *w);
};

#endif // POPUP_H
