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
	_edit->setMinimumWidth(fm.averageCharWidth() * (QDir::homePath().length()
	  + 12));
#ifdef Q_OS_WIN32
	_button = new QPushButton("...");
	_button->setMaximumWidth(_button->sizeHint().width() / 2);
#else // Q_OS_WIN32
	_button = new QToolButton();
	_button->setText("...");
#endif // Q_OS_WIN32
	connect(_button, SIGNAL(clicked()), this, SLOT(browse()));

	QHBoxLayout *layout = new QHBoxLayout();
	layout->setContentsMargins(QMargins());
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

bool FileSelectWidget::checkFile(QString &error) const
{
	if (_edit->text().isEmpty()) {
		error = tr("No output file selected.");
		return false;
	}

	QFile file(_edit->text());
	QFileInfo fi(file);
	bool exists = fi.exists();
	bool opened = false;

	if (exists && fi.isDir()) {
		error = tr("%1 is a directory.").arg(file.fileName());
		return false;
	} else if ((exists && !fi.isWritable())
	  || !(opened = file.open(QFile::Append))) {
		error = tr("%1 is not writable.").arg(file.fileName());
		return false;
	}

	if (opened) {
		file.close();
		if (!exists)
			file.remove();
	}

	return true;
}
