#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QFileInfo>
#include <QMessageBox>
#include <QTabWidget>
#include <QDoubleSpinBox>
#include <QLocale>
#include "fileselectwidget.h"
#include "units.h"
#include "exportdialog.h"


ExportDialog::ExportDialog(QPrinter *printer, QWidget *parent)
  : QDialog(parent), _printer(printer)
{
	int index;

	_units = (QLocale::system().measurementSystem()
	  == QLocale::ImperialSystem) ? QPrinter::Inch : QPrinter::Millimeter;

	_fileSelect = new FileSelectWidget();
	_fileSelect->setFilter(tr("PDF files (*.pdf);;All files (*)"));
	_fileSelect->setFile(_printer->outputFileName());

	_paperSize = new QComboBox();
	_paperSize->addItem("A3", QPrinter::A3);
	_paperSize->addItem("A4", QPrinter::A4);
	_paperSize->addItem("A5", QPrinter::A5);
	_paperSize->addItem("Tabloid", QPrinter::Tabloid);
	_paperSize->addItem("Legal", QPrinter::Legal);
	_paperSize->addItem("Letter", QPrinter::Letter);
	if ((index = _paperSize->findData(_printer->paperSize())) >= 0)
		_paperSize->setCurrentIndex(index);

	_portrait = new QRadioButton(tr("Portrait"));
	_landscape = new QRadioButton(tr("Landscape"));
	QHBoxLayout *orientationLayout = new QHBoxLayout();
	orientationLayout->addWidget(_portrait);
	orientationLayout->addWidget(_landscape);
	if (_printer->orientation() == QPrinter::Portrait)
		_portrait->setChecked(true);
	else
		_landscape->setChecked(true);

	qreal top, bottom, left, right;

	_printer->getPageMargins(&left, &top, &right, &bottom, _units);
	QString us = _units == QPrinter::Inch ? tr("in") : tr("mm");
	_topMargin = new QDoubleSpinBox();
	_bottomMargin = new QDoubleSpinBox();
	_leftMargin = new QDoubleSpinBox();
	_rightMargin = new QDoubleSpinBox();
	_topMargin->setValue(top);
	_topMargin->setSuffix(UNIT_SPACE + us);
	_bottomMargin->setValue(bottom);
	_bottomMargin->setSuffix(UNIT_SPACE + us);
	_leftMargin->setValue(left);
	_leftMargin->setSuffix(UNIT_SPACE + us);
	_rightMargin->setValue(right);
	_rightMargin->setSuffix(UNIT_SPACE + us);
	if (_units == QPrinter::Inch) {
		_topMargin->setSingleStep(0.1);
		_bottomMargin->setSingleStep(0.1);
		_leftMargin->setSingleStep(0.1);
		_rightMargin->setSingleStep(0.1);
	}

	QGridLayout *marginsLayout = new QGridLayout();
	marginsLayout->addWidget(_topMargin, 0, 0, 1, 2, Qt::AlignCenter);
	marginsLayout->addWidget(_leftMargin, 1, 0, 1, 1, Qt::AlignRight);
	marginsLayout->addWidget(_rightMargin, 1, 1, 1, 1, Qt::AlignLeft);
	marginsLayout->addWidget(_bottomMargin, 2, 0, 1, 2, Qt::AlignCenter);

#ifndef Q_OS_MAC
	QGroupBox *pageSetupBox = new QGroupBox(tr("Page Setup"));
#endif // Q_OS_MAC
	QFormLayout *pageSetupLayout = new QFormLayout;
	pageSetupLayout->addRow(tr("Page size:"), _paperSize);
	pageSetupLayout->addRow(tr("Orientation:"), orientationLayout);
	pageSetupLayout->addRow(tr("Margins:"), marginsLayout);
#ifdef Q_OS_MAC
	QFrame *line = new QFrame();
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	pageSetupLayout->addRow(line);
	pageSetupLayout->addRow(tr("File:"), _fileSelect);
	pageSetupLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
#else // Q_OS_MAC
	pageSetupBox->setLayout(pageSetupLayout);
#endif // Q_OS_MAC

#ifndef Q_OS_MAC
	QGroupBox *outputFileBox = new QGroupBox(tr("Output file"));
	QHBoxLayout *outputFileLayout = new QHBoxLayout();
	outputFileLayout->addWidget(_fileSelect);
	outputFileBox->setLayout(outputFileLayout);
#endif // Q_OS_MAC

	QDialogButtonBox *buttonBox = new QDialogButtonBox();
	buttonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);
	buttonBox->addButton(QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout *layout = new QVBoxLayout;
#ifdef Q_OS_MAC
	layout->addLayout(pageSetupLayout);
#else // Q_OS_MAC
	layout->addWidget(pageSetupBox);
	layout->addWidget(outputFileBox);
#endif // Q_OS_MAC
	layout->addWidget(buttonBox);
	setLayout(layout);

	setWindowTitle(tr("Export to PDF"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

bool ExportDialog::checkFile()
{
	if (_fileSelect->file().isEmpty()) {
		QMessageBox::warning(this, tr("Error"), tr("No output file selected."));
		return false;
	}

	QFile file(_fileSelect->file());
	QFileInfo fi(file);
	bool exists = fi.exists();
	bool opened = false;

	if (exists && fi.isDir()) {
		QMessageBox::warning(this, tr("Error"), tr("%1 is a directory.")
		  .arg(file.fileName()));
		return false;
	} else if ((exists && !fi.isWritable())
	  || !(opened = file.open(QFile::Append))) {
		QMessageBox::warning(this, tr("Error"), tr("%1 is not writable.")
		  .arg(file.fileName()));
		return false;
	}
	if (opened) {
		file.close();
		if (!exists)
			file.remove();
	}

	return true;
}

void ExportDialog::accept()
{
	if (!checkFile())
		return;

	QPrinter::Orientation orientation = _portrait->isChecked()
	  ? QPrinter::Portrait : QPrinter::Landscape;
	QPrinter::PaperSize paperSize = static_cast<QPrinter::PaperSize>
	  (_paperSize->itemData(_paperSize->currentIndex()).toInt());

	_printer->setOutputFormat(QPrinter::PdfFormat);
	_printer->setOutputFileName(_fileSelect->file());
	_printer->setPaperSize(paperSize);
	_printer->setOrientation(orientation);
	_printer->setPageMargins(_leftMargin->value(), _topMargin->value(),
	  _rightMargin->value(), _bottomMargin->value(), _units);

	QDialog::accept();
}
