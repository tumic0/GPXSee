#include <QtWidgets>
#include "flowlayout.h"

struct FlowLayoutItem
{
	FlowLayoutItem() : item(0) {}
	FlowLayoutItem(QLayoutItem *item, int x, int y) : item(item), pos(x, y) {}

	QLayoutItem *item;
	QPoint pos;
};

FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), _hSpace(hSpacing), _vSpace(vSpacing)
{
	setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
    : _hSpace(hSpacing), _vSpace(vSpacing)
{
	setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
	qDeleteAll(_items);
}

void FlowLayout::addItem(QLayoutItem *item)
{
	_items.append(item);
}

int FlowLayout::horizontalSpacing() const
{
	return (_hSpace >= 0)
	  ? _hSpace
	  : smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const
{
	return (_vSpace >= 0)
	  ? _vSpace
	  : smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int FlowLayout::count() const
{
	return _items.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
	return _items.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index)
{
	if (index >= 0 && index < _items.size())
		return _items.takeAt(index);

	return 0;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
	return {};
}

bool FlowLayout::hasHeightForWidth() const
{
	return true;
}

int FlowLayout::heightForWidth(int width) const
{
	int height = doLayout(QRect(0, 0, width, 0), true);
	return height;
}

void FlowLayout::setGeometry(const QRect &rect)
{
	QLayout::setGeometry(rect);
	doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const
{
	return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
	QSize size;

	for (int i = 0; i < _items.size(); i++)
		size = size.expandedTo(_items.at(i)->minimumSize());

	const QMargins margins = contentsMargins();
	size += QSize(margins.left() + margins.right(), margins.top()
	  + margins.bottom());

	return size;
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
	int left, top, right, bottom;
	getContentsMargins(&left, &top, &right, &bottom);
	QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
	int x = effectiveRect.x();
	int y = effectiveRect.y();
	int lineHeight = 0;
	QVector<QVector<FlowLayoutItem>> rows;

	for (int i = 0; i < _items.size(); i++) {
		QLayoutItem *item = _items.at(i);
		const QWidget *wid = item->widget();
		int spaceX = horizontalSpacing();
		if (spaceX == -1)
			spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton,
			  QSizePolicy::PushButton, Qt::Horizontal);
		int spaceY = verticalSpacing();
		if (spaceY == -1)
			spaceY = wid->style()->layoutSpacing(QSizePolicy::PushButton,
			  QSizePolicy::PushButton, Qt::Vertical);

		int nextX = x + item->sizeHint().width() + spaceX;
		if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
			x = effectiveRect.x();
			y = y + lineHeight + spaceY;
			nextX = x + item->sizeHint().width() + spaceX;
			lineHeight = 0;
			rows.append(QVector<FlowLayoutItem>());
		}

		if (rows.isEmpty())
			rows.append(QVector<FlowLayoutItem>());
		rows.last().append(FlowLayoutItem(item, x, y));

		x = nextX;
		lineHeight = qMax(lineHeight, item->sizeHint().height());
	}

	if (!testOnly) {
		for (int i = 0; i < rows.size(); i++) {
			const FlowLayoutItem &li = rows.at(i).last();
			int width = li.item->sizeHint().width() + li.pos.x()
			  - effectiveRect.x();
			int offset = (effectiveRect.width() - width) / 2;

			int height = 0;
			for (int j = 0; j < rows.at(i).size(); j++)
				height = qMax(rows.at(i).at(j).item->sizeHint().height(), height);

			for (int j = 0; j < rows.at(i).size(); j++) {
				QLayoutItem *item = rows.at(i).at(j).item;
				const QPoint &p = rows.at(i).at(j).pos;
				QSize sh(item->sizeHint());
				item->setGeometry(QRect(QPoint(p.x() + offset, p.y() + height
				  - sh.height()), sh));
			}
		}
	}

	return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
	QObject *parent = this->parent();
	if (!parent)
		return -1;
	else if (parent->isWidgetType()) {
		QWidget *pw = static_cast<QWidget *>(parent);
		return pw->style()->pixelMetric(pm, 0, pw);
	} else
		return static_cast<QLayout *>(parent)->spacing();
}
