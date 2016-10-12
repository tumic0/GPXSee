#include <QPushButton>
#include <QToolButton>
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
#ifdef Q_OS_WIN32
	_button = new QPushButton("...");
	_button->setMaximumWidth(_button->sizeHint().width() / 2);
#else // Q_OS_WIN32
	_button = new QToolButton();
	_button->setText("...");
#endif // Q_OS_WIN32
	connect(_button, SIGNAL(clicked()), this, SLOT(browse()));

	QHBoxLayout *layout = new QHBoxLayout();
	layout->setMargin(0);
	layout->addWidget(_edit);
	layout->addWidget(_button);
	setLayout(layout);

	QSizePolicy p(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setSizePolicy(p);
}

void FileSelectWidget::browse()
{
	QFileInfo fi(_edit->text());
	QString fileName = QFileDialog::getSaveFileName(this, tr("Select file"),
	  fi.dir().absolutePath(), _filter);

	if (!fileName.isEmpty())
		_edit->setText(fileName);
}
