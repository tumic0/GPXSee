#ifndef PASSWORDEDIT_H
#define PASSWORDEDIT_H

#include <QLineEdit>

class PasswordEdit : public QLineEdit
{
	Q_OBJECT

public:
	PasswordEdit(QWidget *parent = 0);

private slots:
	void showPassword();

private:
	QAction *_action;
	bool _show;
};

#endif // PASSWORDEDIT_H
