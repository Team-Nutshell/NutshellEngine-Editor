#pragma once
#include "ntshengn_networking_common.h"
#include <string>
#include <functional>
#include <unordered_map>

namespace NtshEngn {

	struct ConnectedClient {
		Socket socket; // TCP
		std::string ipAddress; // UDP
		uint16_t port; // UDP & TCP
	};

	class ServerSocketInterface {
	public:
		virtual ~ServerSocketInterface() {}

		virtual void update() = 0;
		virtual void destroy() = 0;

		virtual void sendDataToClient(ConnectedClientID clientID, const void* data, size_t dataSize) = 0;

		virtual void setClientConnectCallback(std::function<void(ConnectedClientID)> callback) = 0;
		virtual void setClientDisconnectCallback(std::function<void(ConnectedClientID)> callback) = 0;
		virtual void setDataReceivedCallback(std::function<void(ConnectedClientID, void*, size_t)> callback) = 0;

		virtual uint16_t getPort() = 0;
		virtual const std::unordered_map<ConnectedClientID, ConnectedClient>& getConnectedClients() = 0;
	};

}