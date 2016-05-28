#include <QPushButton>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QApplication>
#include <QFontMetrics>
#include "fileselectwidget.h"


FileSelectWidget::FileSelectWidget(QWidget *parent) : QWidget(parent)
{
	QFontMetrics fm(QApplication::font());
	_edit = new QLineEdit();
	_edit->setMinimumWidth(fm.boundingRect(QDir::homePath()).width());
	_button = new QPushButton(tr("Browse..."));
	connect(_button, SIGNAL(clicked()), this, SLOT(browse()));

	QHBoxLayout *layout = new QHBoxLayout();
	layout->setMargin(0);
	layout->addWidget(_edit);
	layout->addWidget(_button);
	setLayout(layout);
}

void FileSelectWidget::browse()
{
	QFileInfo fi(_edit->text());
	QString fileName = QFileDialog::getSaveFileName(this, tr("Select file"),
	  fi.dir().absolutePath(), _filter);

	if (!fileName.isEmpty())
		_edit->setText(fileName);
}
