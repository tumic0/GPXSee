#ifndef POPUP_H
#define POPUP_H

class QPoint;
class QWidget;
class ToolTip;

class Popup
{
public:
	static void show(const QPoint &pos, const ToolTip &toolTip, QWidget *w);
	static void clear();
};

#endif // POPUP_H
