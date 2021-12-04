#include <QFormLayout>
#include <QLineEdit>
#include "pluginparameters.h"

static const QMap<QString, QStringList> pluginParams = {
	{"nmea", {"nmea.source"}},
	{"serialnmea", {"serialnmea.serial_port"}},
	{"geoclue2", {"desktopId"}}
};

static void deleteLayout(QLayout *layout)
{
	if (!layout)
		return;

	while (layout->count() > 0) {
		QLayoutItem *child = layout->takeAt(0);
		deleteLayout(child->layout());
		delete child->widget();
		delete child;
	}

	delete layout;
}

PluginParameters::PluginParameters(const QString &plugin,
  const QMap<QString, QVariantMap> &params, QWidget *parent)
  : QWidget(parent), _params(params), _plugin(plugin)
{
	setPlugin(plugin);
}

void PluginParameters::setPlugin(const QString &plugin)
{
	saveParameters();

	QStringList params = pluginParams.value(plugin);

	QFormLayout *l = new QFormLayout();
	for (int i = 0; i < params.size(); i++) {
		QLineEdit *le = new QLineEdit();
		le->setObjectName(params.at(i));
		le->setText(_params[plugin].value(params.at(i)).toString());
		l->addRow(params.at(i) + ":", le);
		connect(le, &QLineEdit::editingFinished, this,
		  &PluginParameters::saveParameters);
	}

	deleteLayout(layout());
	setLayout(l);

	_plugin = plugin;
}

void PluginParameters::saveParameters()
{
	QVariantMap &map(_params[_plugin]);
	QFormLayout *l = qobject_cast<QFormLayout*>(layout());
	if (!l)
		return;

	for (int i = 0; i < l->rowCount(); i++) {
		QLayoutItem *li = l->itemAt(i, QFormLayout::FieldRole);
		QLineEdit *le = qobject_cast<QLineEdit*>(li->widget());
		if (le) {
			if (le->text().isEmpty())
				map.remove(le->objectName());
			else
				map.insert(le->objectName(), le->text());
		}
	}
}
