#pragma once
#include "Socket/Socket.hpp"

using namespace SERVER::FUNCTIONS::SOCKETADDRESS;

namespace SERVER {
	namespace NETWORK {
		namespace PROTOCOL {
			namespace UTIL {
				namespace UDP {
					bool SendTo(const ::SOCKET hSocket, const SocketAddress& sendAddress, const char* const sSendBuffer, const uint16_t iDataLength);
					bool ReceiveFrom(const ::SOCKET hSocket, char* const sReceiveBuffer, uint16_t& iReceiveBytes, SERVER::NETWORK::SESSION::SERVERSESSION::OVERLAPPED_EX& receiveOverlapped);
				}
			}

			namespace UDP {

				class UDPIPSocket : public PROTOCOL::SOCKET::BaseSocket {
				public:
					UDPIPSocket();
					virtual ~UDPIPSocket() override;

				public:
					bool WriteTo(const SocketAddress& sendAddress, const char* const sSendData, const uint16_t iDataLength);

					bool ReadFrom(char* const sReceiveBuffer, uint16_t& iRecvBytes);
					

				};
			}
		}
	}
}