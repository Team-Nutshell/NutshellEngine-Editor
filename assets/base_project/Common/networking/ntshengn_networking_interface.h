#pragma once
#include "ntshengn_networking_server_socket_interface.h"
#include "ntshengn_networking_client_socket_interface.h"

namespace NtshEngn {

	class NetworkingInterface {
	public:
		virtual void init() = 0;
		virtual void update() = 0;
		virtual void destroy() = 0;

		virtual ServerSocketInterface* createServerSocket(uint16_t port, NetworkType networkType) = 0;
		virtual ClientSocketInterface* createClientSocket(NetworkType networkType) = 0;

		virtual void closeServerSocket(ServerSocketInterface* serverSocket) = 0;
		virtual void closeClientSocket(ClientSocketInterface* clientSocket) = 0;
	};

}