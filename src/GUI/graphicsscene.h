#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneHelpEvent>
#include <QWidget>
#include "popup.h"

class GraphicsItem : public QGraphicsItem
{
public:
	GraphicsItem(QGraphicsItem *parent = 0) : QGraphicsItem(parent) {}

	virtual QString info() const = 0;
	int type() const {return QGraphicsItem::UserType + 1;}
};

class GraphicsScene : public QGraphicsScene
{
public:
	GraphicsScene(QObject *parent = 0) : QGraphicsScene(parent) {}

protected:
	void helpEvent(QGraphicsSceneHelpEvent *event)
	{
		QGraphicsItem *item = itemAt(event->scenePos(), QTransform());
		if (item && item->type() == QGraphicsItem::UserType + 1) {
			GraphicsItem *mi = static_cast<GraphicsItem*>(item);
			Popup::show(event->screenPos(), mi->info(),
			  static_cast<QWidget*>(parent()));
		}
	}
};

#endif // GRAPHICSSCENE_H
