#include <QPushButton>
#include <QToolButton>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QApplication>
#include <QFontMetrics>
#include "common/util.h"
#include "fileselectwidget.h"


FileSelectWidget::FileSelectWidget(QWidget *parent) : QWidget(parent)
{
	_edit = new QLineEdit();
#ifdef Q_OS_ANDROID
	_edit->setDisabled(true);
#else // Q_OS_ANDROID
	QFontMetrics fm(QApplication::font());
	_edit->setMinimumWidth(fm.averageCharWidth() * (QDir::homePath().length()
	  + 12));
#endif // Q_OS_ANDROID
#ifdef Q_OS_WIN32
	_button = new QPushButton("...");
	_button->setMaximumWidth(_button->sizeHint().width() / 2);
#else // Q_OS_WIN32
	_button = new QToolButton();
	_button->setText("...");
#endif // Q_OS_WIN32
	connect(_button, &QToolButton::clicked, this, &FileSelectWidget::browse);

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
#ifdef Q_OS_ANDROID
	_fileName = QFileDialog::getSaveFileName(this, tr("Select file"));
	if (!_fileName.isEmpty())
		_edit->setText(Util::displayName(_fileName));
#else // Q_OS_ANDROID
	QFileInfo fi(_edit->text());
	QString fileName = QFileDialog::getSaveFileName(this, tr("Select file"),
	  fi.dir().absolutePath(), _filter);
	if (!fileName.isEmpty())
		_edit->setText(fileName);
#endif // Q_OS_ANDROID
}

bool FileSelectWidget::checkFile(QString &error) const
{
	if (file().isEmpty()) {
		error = tr("No output file selected.");
		return false;
	}

	QFile f(file());
	QFileInfo fi(f);
	bool exists = fi.exists();
	bool opened = false;

	if (exists && fi.isDir()) {
		error = tr("%1 is a directory.").arg(f.fileName());
		return false;
	} else if ((exists && !fi.isWritable())
	  || !(opened = f.open(QFile::Append))) {
		error = tr("%1 is not writable.").arg(f.fileName());
		return false;
	}

	if (opened) {
		f.close();
		if (!exists)
			f.remove();
	}

	return true;
}
