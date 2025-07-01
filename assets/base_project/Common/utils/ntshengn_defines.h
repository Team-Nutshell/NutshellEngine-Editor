#pragma once
#include <iostream>
#include <filesystem>
#include <string>

#if defined(NTSHENGN_DEBUG)
#if defined(NTSHENGN_COMPILER_MSVC)
#define NTSHENGN_BREAKPOINT() \
	do { \
		__debugbreak(); \
	} while(0)
#elif defined(NTSHENGN_COMPILER_GCC) || defined(NTSHENGN_COMPILER_CLANG)
#define NTSHENGN_BREAKPOINT() \
	do { \
		__builtin_trap(); \
	} while(0)
#endif
#else
#define NTSHENGN_BREAKPOINT() \
	do { \
	} while(0)
#endif

#define NTSHENGN_UNUSED(expression) \
	do { \
		(void)(expression); \
	} while(0)

#if defined(NTSHENGN_DEBUG)
#define NTSHENGN_ASSERT(expression, message) \
	do { \
		if (!(expression)) { \
			std::cout << "\33[1m\33[31mASSERTION FAILED\33[39m\33[0m: " + std::string(#expression) + " (" + message + ")" + "\nLine \33[1m" + std::to_string(__LINE__) + "\33[0m in file \33[1m" + std::filesystem::path(__FILE__).filename().string() + "\33[0m." << std::endl; \
			NTSHENGN_BREAKPOINT(); \
			exit(1); \
		} \
	} while(0)
#else
#define NTSHENGN_ASSERT(expression, message) \
	do { \
	} while(0)
#endif

#define NTSHENGN_POINTER_EXECUTE(pointer, function) \
	do { \
		if (pointer) { \
			pointer->function; \
		} \
	} while(0)
