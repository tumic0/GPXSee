#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0) || defined(Q_OS_MAC)
#include <QGLWidget>
#include <QGLFormat>
#else
#include <QOpenGLWidget>
#include <QSurfaceFormat>
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0) || defined(Q_OS_MAC)
#define OPENGL_WIDGET QGLWidget
#else
#define OPENGL_WIDGET QOpenGLWidget
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0) || defined(Q_OS_MAC)
#define OPENGL_SET_SAMPLES(samples) \
	{QGLFormat fmt; \
	fmt.setSamples(samples); \
	QGLFormat::setDefaultFormat(fmt);}
#else
#define OPENGL_SET_SAMPLES(samples) \
	{QSurfaceFormat fmt; \
	fmt.setSamples(samples);\
	QSurfaceFormat::setDefaultFormat(fmt);}
#endif
