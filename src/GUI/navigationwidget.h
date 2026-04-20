#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include <QWidget>

class MapView;

#ifdef Q_OS_ANDROID
class NavigationWidget : public QWidget
{
	Q_OBJECT

public:
	NavigationWidget(QWidget *parent = 0);

	void showNext(bool enable) {_showNext = enable; update();}
	void showPrev(bool enable) {_showPrev = enable; update();}

	bool pressed(const QPoint &pos);
	bool released(const QPoint &pos);

signals:
	void menu(const QPoint &pos);
	void next();
	void prev();

private:
	bool eventFilter(QObject *obj, QEvent *ev);
	bool event(QEvent *ev);
	void paintEvent(QPaintEvent *ev);
	void newParent();

	bool _menuHover, _prevHover, _nextHover;
	bool _showPrev, _showNext;
};
#endif // Q_OS_ANDROID

#endif // NAVIGATIONWIDGET_H
