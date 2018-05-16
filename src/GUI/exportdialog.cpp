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
#include "fileselectwidget.h"
#include "units.h"
#include "exportdialog.h"


ExportDialog::ExportDialog(Export *exp, QWidget *parent)
  : QDialog(parent), _export(exp)
{
	int index;

	_fileSelect = new FileSelectWidget();
	_fileSelect->setFilter(tr("PDF files (*.pdf);;All files (*)"));
	_fileSelect->setFile(_export->fileName);

	_paperSize = new QComboBox();
	_paperSize->addItem("A2", QPrinter::A2);
	_paperSize->addItem("A3", QPrinter::A3);
	_paperSize->addItem("A4", QPrinter::A4);
	_paperSize->addItem("A5", QPrinter::A5);
	_paperSize->addItem("A6", QPrinter::A6);
	_paperSize->addItem("B3", QPrinter::B3);
	_paperSize->addItem("B4", QPrinter::B4);
	_paperSize->addItem("B5", QPrinter::B5);
	_paperSize->addItem("B6", QPrinter::B6);
	_paperSize->addItem("Tabloid", QPrinter::Tabloid);
	_paperSize->addItem("Legal", QPrinter::Legal);
	_paperSize->addItem("Letter", QPrinter::Letter);
	if ((index = _paperSize->findData(_export->paperSize)) >= 0)
		_paperSize->setCurrentIndex(index);

	_resolution = new QComboBox();
	_resolution->addItem("150 DPI", 150);
	_resolution->addItem("300 DPI", 300);
	_resolution->addItem("600 DPI", 600);
	if ((index = _resolution->findData(_export->resolution)) >= 0)
		_resolution->setCurrentIndex(index);

	_portrait = new QRadioButton(tr("Portrait"));
	_landscape = new QRadioButton(tr("Landscape"));
	QHBoxLayout *orientationLayout = new QHBoxLayout();
	orientationLayout->addWidget(_portrait);
	orientationLayout->addWidget(_landscape);
	if (_export->orientation == QPrinter::Portrait)
		_portrait->setChecked(true);
	else
		_landscape->setChecked(true);

	_topMargin = new QDoubleSpinBox();
	_bottomMargin = new QDoubleSpinBox();
	_leftMargin = new QDoubleSpinBox();
	_rightMargin = new QDoubleSpinBox();
	QString us = (_export->units == Metric) ? tr("mm") : tr("in");
	_topMargin->setSuffix(UNIT_SPACE + us);
	_bottomMargin->setSuffix(UNIT_SPACE + us);
	_leftMargin->setSuffix(UNIT_SPACE + us);
	_rightMargin->setSuffix(UNIT_SPACE + us);
	if (_export->units == Metric) {
		_topMargin->setValue(_export->margins.top());
		_bottomMargin->setValue(_export->margins.bottom());
		_leftMargin->setValue(_export->margins.left());
		_rightMargin->setValue(_export->margins.right());
	} else {
		_topMargin->setValue(_export->margins.top() * MM2IN);
		_bottomMargin->setValue(_export->margins.bottom() * MM2IN);
		_leftMargin->setValue(_export->margins.left() * MM2IN);
		_rightMargin->setValue(_export->margins.right() * MM2IN);
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
	pageSetupLayout->addRow(tr("Resolution:"), _resolution);
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
	int resolution = _resolution->itemData(_resolution->currentIndex()).toInt();

	_export->fileName = _fileSelect->file();
	_export->paperSize = paperSize;
	_export->resolution = resolution;
	_export->orientation = orientation;
	if (_export->units == Imperial)
		_export->margins = MarginsF(_leftMargin->value() / MM2IN,
		_topMargin->value() / MM2IN, _rightMargin->value() / MM2IN,
		_bottomMargin->value() / MM2IN);
	else
		_export->margins = MarginsF(_leftMargin->value(), _topMargin->value(),
		  _rightMargin->value(), _bottomMargin->value());

	QDialog::accept();
}
