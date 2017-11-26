#ifndef CPUARCH_H
#define CPUARCH_H

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)

#if defined(__arm64__)
	#define CPU_ARCH_STR "arm64"
#elif defined(__arm__) || defined(__TARGET_ARCH_ARM) || defined(_M_ARM)
	#define CPU_ARCH_STR "arm"
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) \
 || defined(_M_X64)
	#define CPU_ARCH_STR "x86_64"
#elif defined(__i386) || defined(__i386__) || defined(_M_IX86)
	#define CPU_ARCH_STR "i386"
#elif defined(__ia64) || defined(__ia64__) || defined(_M_IA64)
	#define CPU_ARCH_STR "ia64"
#elif defined(_MIPS_ARCH_MIPS64) || defined(__mips64)
	#define CPU_ARCH_STR "mips64"
#elif defined(__mips) || defined(__mips__) || defined(_M_MRX000)
	#define CPU_ARCH_STR "mips"
#elif defined(__ppc64__) || defined(__powerpc64__) || defined(__64BIT__)
	#define CPU_ARCH_STR "power64"
#elif defined(__ppc__) || defined(__ppc) || defined(__powerpc__) \
  || defined(_ARCH_COM) || defined(_ARCH_PWR) || defined(_ARCH_PPC)  \
  || defined(_M_MPPC) || defined(_M_PPC)
	#define CPU_ARCH_STR "power"
#else
	#define CPU_ARCH_STR "unknown"
#endif

#define CPU_ARCH QString(CPU_ARCH_STR)

#else // QT_VERSION < 5.4

#include <QSysInfo>
#define CPU_ARCH QSysInfo::buildCpuArchitecture()

#endif // QT_VERSION < 5.4

#endif // CPUARCH_H
