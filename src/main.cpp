#include "common/config.h"
#ifdef ENABLE_TIMEZONES
#include "GUI/timezoneinfo.h"
#endif // ENABLE_TIMEZONES
#include "GUI/app.h"

int main(int argc, char *argv[])
{
#ifdef ENABLE_HIDPI
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif // ENABLE_HIDPI
#ifdef ENABLE_TIMEZONES
	qRegisterMetaTypeStreamOperators<TimeZoneInfo>("TimeZoneInfo");
#endif // ENABLE_TIMEZONES

	App app(argc, argv);
	return app.run();
}
