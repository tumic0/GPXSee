#include <QPushButton>
#include <QFileDialog>
#include <QHBoxLayout>
#include "fileselectwidget.h"


FileSelectWidget::FileSelectWidget(QWidget *parent) : QWidget(parent)
{
	_edit = new QLineEdit();
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
	QString fileName = QFileDialog::getSaveFileName(this, tr("Select file"),
	  QDir::currentPath(), _filter);

	if (!fileName.isEmpty())
		_edit->setText(fileName);
}
