#ifndef AUTHENTICATIONWIDGET_H
#define AUTHENTICATIONWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include "passwordedit.h"

class AuthenticationWidget : public QWidget
{
	Q_OBJECT

public:
	AuthenticationWidget(QWidget *parent = 0);

	QString username() const {return _username->text();}
	QString password() const {return _password->text();}

	void setUsername(const QString &username) {_username->setText(username);}
	void setPassword(const QString &password) {_password->setText(password);}

private:
	QLineEdit *_username;
	PasswordEdit *_password;
};

#endif // AUTHENTICATIONWIDGET_H
