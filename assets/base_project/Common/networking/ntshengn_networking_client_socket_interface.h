#pragma once
#include "ntshengn_networking_common.h"
#include <functional>
#include <string>

namespace NtshEngn {
	struct ConnectedServer {
		std::string ipAddress = ""; // UDP
		uint16_t port; // UDP & TCP
	};

	class ClientSocketInterface {
	public:
		virtual ~ClientSocketInterface() {}

		virtual void update() = 0;
		virtual void destroy() = 0;

		virtual void connectToServer(const std::string& ipAddress, uint16_t port) = 0;

		virtual void sendDataToServer(const void* data, size_t dataSize) = 0;

		virtual void setServerDisconnectCallback(std::function<void()> callback) = 0;
		virtual void setDataReceivedCallback(std::function<void(void*, size_t)> callback) = 0;

		virtual const ConnectedServer& getConnectedServer() = 0;
	};

}