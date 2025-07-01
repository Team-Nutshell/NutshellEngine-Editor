#pragma once
#include <iostream>
#include <string>
#include <cstdint>

#define NTSHENGN_NETWORKING_INFO(message) \
	do { \
		std::cout << "\33[1m\33[93mNETWORKING \33[34mINFO\33[39m\33[0m: " + std::string(message) << std::endl; \
	} while(0)

#define NTSHENGN_NETWORKING_WARNING(message) \
	do { \
		std::cout << "\33[1m\33[93mNETWORKING \33[93mWARNING\33[39m\33[0m: " + std::string(message) << std::endl; \
	} while(0)

#define NTSHENGN_NETWORKING_ERROR(message, code) \
	do { \
		std::cout << "\33[1m\33[93mNETWORKING \33[31mERROR\33[39m\33[0m: " + std::string(message) + " (" + std::string(#code) + ")" << std::endl; \
		exit(1); \
	} while(0)

#define NTSHENGN_NETWORKING_BUFFER_SIZE 65536

#define NTSHENGN_NETWORKING_HEADER_CONNECTION 0xC044
#define NTSHENGN_NETWORKING_HEADER_DISCONNECTION 0xD15C

namespace NtshEngn {

	typedef uint64_t Socket;

	typedef uint32_t ConnectedClientID;

	enum class NetworkType {
		UDP,
		TCP
	};

}