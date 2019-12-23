#include <QGraphicsSceneHelpEvent>
#include <QGraphicsView>
#include "popup.h"
#include "graphicsscene.h"


/* Standard GraphicsScene::items() is not pixel accurate, so we use the
   following function which has the same logic as used in the original
   QGraphicsScene::helpEvent() function. */
QList<QGraphicsItem *> GraphicsScene::itemsAtPosition(const QPoint &screenPos,
  const QPointF &scenePos, QWidget *widget) const
{
	QGraphicsView *view = widget
	  ? qobject_cast<QGraphicsView *>(widget->parentWidget()) : 0;

	if (!view)
		return items(scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder,
		  QTransform());

	const QRectF pointRect(QPointF(widget->mapFromGlobal(screenPos)),
	  QSizeF(1, 1));
	if (!view->isTransformed())
		return items(pointRect, Qt::IntersectsItemShape, Qt::DescendingOrder);

	const QTransform viewTransform = view->viewportTransform();
	if (viewTransform.type() <= QTransform::TxScale)
		return items(viewTransform.inverted().mapRect(pointRect),
		  Qt::IntersectsItemShape, Qt::DescendingOrder, viewTransform);

	return items(viewTransform.inverted().map(pointRect),
	  Qt::IntersectsItemShape, Qt::DescendingOrder, viewTransform);
}

void GraphicsScene::helpEvent(QGraphicsSceneHelpEvent *event)
{
	QList<QGraphicsItem *> list = itemsAtPosition(event->screenPos(),
	  event->scenePos(), event->widget());

	for (int i = 0; i < list.size(); i++) {
		if (list.at(i)->type() == QGraphicsItem::UserType + 1) {
			GraphicsItem *mi = static_cast<GraphicsItem*>(list.at(i));
			Popup::show(event->screenPos(), mi->info(), event->widget());
			return;
		}
	}

	/* No need to process QGraphicsScene::helpEvent() */
}

void GraphicsScene::clear()
{
	Popup::clear();
	QGraphicsScene::clear();
}
