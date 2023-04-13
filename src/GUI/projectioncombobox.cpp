#include "projectioncombobox.h"

ProjectionComboBox::ProjectionComboBox(const QList<KV<int, QString> > &list,
  QWidget *parent) : QComboBox(parent)
{
	setSizeAdjustPolicy(AdjustToMinimumContentsLengthWithIcon);
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	for (int i = 0; i < list.size(); i++) {
		const KV<int, QString> &proj = list.at(i);
		QString text = QString::number(proj.key()) + " - " + proj.value();
		addItem(text, QVariant(proj.key()));
	}
}
