#include <QPushButton>
#include <QToolButton>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QFontMetrics>
#include <QApplication>
#include "dirselectwidget.h"

DirSelectWidget::DirSelectWidget(QWidget *parent) : QWidget(parent)
{
	QFontMetrics fm(QApplication::font());
	_edit = new QLineEdit();
	_edit->setMinimumWidth(fm.averageCharWidth() * (QDir::homePath().length()
	  + 12));
	_edit->setPlaceholderText(tr("System default"));
#ifdef Q_OS_WIN32
	_button = new QPushButton("...");
	_button->setMaximumWidth(_button->sizeHint().width() / 2);
#else // Q_OS_WIN32
	_button = new QToolButton();
	_button->setText("...");
#endif // Q_OS_WIN32
	connect(_button, &QToolButton::clicked, this, &DirSelectWidget::browse);

	QHBoxLayout *layout = new QHBoxLayout();
	layout->setContentsMargins(QMargins());
	layout->addWidget(_edit);
	layout->addWidget(_button);
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
