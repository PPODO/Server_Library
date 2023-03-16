#pragma once
#include "../Socket.hpp"
#include "../../Functions/CircularQueue/CircularQueue.hpp"
#include <memory>

namespace SERVER {
	namespace NETWORK {
		namespace SESSION {
			namespace SERVERSESSION {
				struct OVERLAPPED_EX;
			}
		}

		namespace SOCKET {
			namespace TCPIP {
				class TCPIPSocket : public SOCKET::BaseSocket {
				public:
					TCPIPSocket();
					virtual ~TCPIPSocket() override;

				public:
					bool Listen(const int32_t iBackLogCount = SOMAXCONN);
					bool Connect(const FUNCTIONS::SOCKETADDRESS::SocketAddress& connectAddress);
					bool Accept(const TCPIPSocket& listenSocket, SESSION::SERVERSESSION::OVERLAPPED_EX& acceptOverlapped);

				public:


				};
			}
		}

		namespace UTIL {
			namespace TCPIP {
				bool Send(const ::SOCKET& hSocket, char* const sSendBuffer, const uint16_t iSendBufferSize);
				bool Receive(const ::SOCKET& hSocket, char* const sReceiveBuffer, uint16_t& iReceiveBufferSize);
			}
		}
	}
}