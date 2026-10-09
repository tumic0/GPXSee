#ifndef POPUP_H
#define POPUP_H

#include <QFrame>
#include <QBasicTimer>
#include "tooltip.h"

class Popup
{
public:
	static void show(const QPoint &pos, const ToolTip &toolTip, QWidget *w);
	static void clear();
};

class PopupFrame : public QFrame
{
	Q_OBJECT

public:
	PopupFrame(const ToolTip &toolTip, QWidget *parent = 0);

	const ToolTip &toolTip() const {return _toolTip;}

	bool eventFilter(QObject *o, QEvent *ev);
	void place(const QPoint &pos, QWidget *w);
	void deleteAfterTimer();
	void stopTimer() {_timer.stop();}

	static PopupFrame *_instance;

protected:
	void paintEvent(QPaintEvent *event);
	void timerEvent(QTimerEvent *event);
	void contextMenuEvent(QContextMenuEvent *) {}

private:
	void createLayout(const ToolTip &content);

	QBasicTimer _timer;
	ToolTip _toolTip;
};

#endif // POPUP_H
