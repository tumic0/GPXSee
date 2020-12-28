#include "GUI/app.h"
#include "GUI/timezoneinfo.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	qRegisterMetaTypeStreamOperators<TimeZoneInfo>("TimeZoneInfo");
#else // QT6
	qRegisterMetaType<TimeZoneInfo>("TimeZoneInfo");
#endif // QT6

	App app(argc, argv);
	return app.run();
}
