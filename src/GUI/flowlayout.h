#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <QLayout>
#include <QRect>
#include <QStyle>

class FlowLayout : public QLayout
{
public:
	FlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1,
	  int vSpacing = -1);
	FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
	~FlowLayout();

	void addItem(QLayoutItem *item);
	int horizontalSpacing() const;
	int verticalSpacing() const;
	Qt::Orientations expandingDirections() const;
	bool hasHeightForWidth() const;
	int heightForWidth(int) const;
	int count() const;
	QLayoutItem *itemAt(int index) const;
	QSize minimumSize() const;
	void setGeometry(const QRect &rect);
	QSize sizeHint() const;
	QLayoutItem *takeAt(int index);

private:
	int doLayout(const QRect &rect, bool testOnly) const;
	int smartSpacing(QStyle::PixelMetric pm) const;

	QList<QLayoutItem *> _items;
	int _hSpace;
	int _vSpace;
};

#endif // FLOWLAYOUT_H
