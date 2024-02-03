#include <QtGlobal>
#include <QSurfaceFormat>
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

	QSurfaceFormat fmt;
	fmt.setProfile(QSurfaceFormat::CoreProfile);
#ifdef Q_OS_ANDROID
	fmt.setRenderableType(QSurfaceFormat::OpenGLES);
#else // Android
	fmt.setVersion(3, 2);
	fmt.setRenderableType(QSurfaceFormat::OpenGL);
#endif // Android
	fmt.setDepthBufferSize(24);
	fmt.setStencilBufferSize(8);
	fmt.setSamples(4);
	QSurfaceFormat::setDefaultFormat(fmt);

	App app(argc, argv);
	return app.run();
}
