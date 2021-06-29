#include "map/pcs.h"
#include "projectioncombobox.h"

ProjectionComboBox::ProjectionComboBox(QWidget *parent) : QComboBox(parent)
{
	setSizeAdjustPolicy(AdjustToMinimumContentsLengthWithIcon);
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	int last = -1;
	QList<KV<int, QString> > projections(GCS::list() + PCS::list());
	std::sort(projections.begin(), projections.end());
	for (int i = 0; i < projections.size(); i++) {
		const KV<int, QString> &proj = projections.at(i);
		// There may be duplicit EPSG codes with different names
		if (proj.key() == last)
			continue;
		else
			last = proj.key();
		QString text = QString::number(proj.key()) + " - " + proj.value();
		addItem(text, QVariant(proj.key()));
	}
}
