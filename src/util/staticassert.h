#ifndef ASSERT_H
#define ASSERT_H

template<bool> struct CompileTimeAssert;
template<> struct CompileTimeAssert <true> {};

#define STATIC_ASSERT(e) \
	(CompileTimeAssert <(e) != 0>())

#endif // ASSERT_H

