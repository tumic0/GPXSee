#ifndef APP_H
#define APP_H

#include <QApplication>

class GUI;

class App : public QApplication
{
	Q_OBJECT

public:
	App(int &argc, char **argv);
	~App();
	int run();

protected:
	bool event(QEvent *event);

#ifdef Q_OS_ANDROID
private slots:
	void appStateChanged(Qt::ApplicationState state);
#endif // Q_OS_ANDROID

private:
	void loadDatums();
	void loadPCSs();

	GUI *_gui;
};

#endif // APP_H
