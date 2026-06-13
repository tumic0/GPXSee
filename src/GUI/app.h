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

private slots:
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0) \
  && !defined(Q_OS_WIN32) && !defined(Q_OS_ANDROID)
	void colorSchemeChanged(Qt::ColorScheme colorScheme);
#endif // QT 6.5 && !Q_OS_WIN32
#ifdef Q_OS_ANDROID
	void appStateChanged(Qt::ApplicationState state);
#endif // Q_OS_ANDROID

private:
	void loadDatums();
	void loadPCSs();

	GUI *_gui;
};

#endif // APP_H
