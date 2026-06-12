#include <QFormLayout>
#include "macos.h"
#include "authenticationwidget.h"

AuthenticationWidget::AuthenticationWidget(QWidget *parent) : QWidget(parent)
{
	_username = new QLineEdit();
	_password = new PasswordEdit();

	if (MacOS::match(style())) {
		/* A hack to fix the issue with different field sizes */
		_username->setMinimumWidth(150);
		_password->setMinimumWidth(150);
	}

	QFormLayout *layout = new QFormLayout();
	layout->addRow(tr("Username:"), _username);
	layout->addRow(tr("Password:"), _password);
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);
}
