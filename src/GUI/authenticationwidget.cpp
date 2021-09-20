#include <QFormLayout>
#include "authenticationwidget.h"

AuthenticationWidget::AuthenticationWidget(QWidget *parent) : QWidget(parent)
{
	_username = new QLineEdit();
	_password = new PasswordEdit();

	QFormLayout *layout = new QFormLayout();
	layout->addRow(tr("Username:"), _username);
	layout->addRow(tr("Password:"), _password);
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);
}
