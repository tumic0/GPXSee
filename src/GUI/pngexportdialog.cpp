#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QTabWidget>
#include <QCheckBox>
#include "units.h"
#include "fileselectwidget.h"
#include "marginswidget.h"
#include "pngexportdialog.h"


PNGExportDialog::PNGExportDialog(PNGExport &exp, QWidget *parent)
  : QDialog(parent), _export(exp)
{
	_fileSelect = new FileSelectWidget();
	_fileSelect->setFilter(tr("PNG files") + " (*.png);;" + tr("All files")
	  + " (*)");
	_fileSelect->setFile(_export.fileName);

	_width = new QSpinBox();
	_width->setMinimum(256);
	_width->setMaximum(4096);
	_width->setValue(_export.size.width());
	_width->setSuffix(UNIT_SPACE + tr("px"));
	_height = new QSpinBox();
	_height->setMinimum(256);
	_height->setMaximum(4096);
	_height->setValue(_export.size.height());
	_height->setSuffix(UNIT_SPACE + tr("px"));

	_margins = new MarginsWidget();
	_margins->setValue(_export.margins);
	_margins->setUnits(tr("px"));

	_antialiasing = new QCheckBox(tr("Use anti-aliasing"));
	_antialiasing->setChecked(_export.antialiasing);

#ifndef Q_OS_MAC
	QGroupBox *pageSetupBox = new QGroupBox(tr("Image Setup"));
#endif // Q_OS_MAC
	QFormLayout *pageSetupLayout = new QFormLayout;
	pageSetupLayout->addRow(tr("Image width:"), _width);
	pageSetupLayout->addRow(tr("Image height:"), _height);
	pageSetupLayout->addRow(tr("Margins:"), _margins);
	pageSetupLayout->addWidget(_antialiasing);
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
	QVBoxLayout *outputFileLayout = new QVBoxLayout();
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

	setWindowTitle(tr("Export to PNG"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void PNGExportDialog::accept()
{
	QString error;
	if (!_fileSelect->checkFile(error)) {
		QMessageBox::warning(this, tr("Error"), error);
		return;
	}

	_export.fileName = _fileSelect->file();
	_export.size = QSize(_width->value(), _height->value());
	_export.margins = _margins->value();
	_export.antialiasing = _antialiasing->isChecked();

	QDialog::accept();
}
