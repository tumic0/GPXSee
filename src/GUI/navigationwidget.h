#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#ifdef Q_OS_ANDROID
#include <QWidget>
#include "mapview.h"

class NavigationWidget : public QWidget
{
	Q_OBJECT

public:
	NavigationWidget(MapView *view);

	void enableNext(bool enable) {_showNext = enable; update();}
	void enablePrev(bool enable) {_showPrev = enable; update();}

signals:
	void next();
	void prev();

private slots:
	void viewClicked(const QPoint &pos);

private:
	bool eventFilter(QObject *obj, QEvent *ev);
	bool event(QEvent *ev);
	void paintEvent(QPaintEvent *ev);
	void newParent();

	bool _showPrev, _showNext;
};
#endif // Q_OS_ANDROID

#endif // NAVIGATIONWIDGET_H
