#include <QPushButton>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QFontMetrics>
#include <QApplication>
#include "dirselectwidget.h"

DirSelectWidget::DirSelectWidget(QWidget *parent) : QWidget(parent)
{
	_edit = new QLineEdit();
#ifndef Q_OS_ANDROID
	QFontMetrics fm(QApplication::font());
	_edit->setMinimumWidth(fm.averageCharWidth() * (QDir::homePath().length()
	  + 12));
#endif // Q_OS_ANDROID
	_edit->setPlaceholderText(tr("System default"));

	QPushButton *button = new QPushButton("...");
	button->setMaximumWidth(35);
	connect(button, &QPushButton::clicked, this, &DirSelectWidget::browse);

	QHBoxLayout *layout = new QHBoxLayout();
	layout->setContentsMargins(QMargins());
	layout->addWidget(_edit);
	layout->addWidget(button);
	setLayout(layout);

	QSizePolicy p(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setSizePolicy(p);
}

void DirSelectWidget::browse()
{
	QString dir(QFileDialog::getExistingDirectory(this, tr("Select directory"),
	  _edit->text()));

	if (!dir.isEmpty())
		_edit->setText(dir);
}
