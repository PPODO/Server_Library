#pragma once
#include "../Functions/SocketAddress/SocketAddress.hpp"
#include <memory>

namespace SERVER {
	namespace NETWORK {
		namespace UTIL {
			namespace SOCKET {
				enum class EPROTOCOLTYPE {
					EPT_TCP,
					EPT_UDP
				};

				static ::SOCKET CreateSocketByProtocolType(const EPROTOCOLTYPE protocolType) {
					::SOCKET hNewSocket = INVALID_SOCKET;
					int iSockType = SOCK_STREAM;
					int iProtocolType = IPPROTO_TCP;

					if (protocolType == EPROTOCOLTYPE::EPT_UDP) {
						iSockType = SOCK_DGRAM;
						iProtocolType = IPPROTO_UDP;
					}

					hNewSocket = WSASocketA(AF_INET, iSockType, iProtocolType, nullptr, 0, WSA_FLAG_OVERLAPPED);
					return hNewSocket;
				}
			}

		}

		namespace SOCKET {
			static const size_t MAX_RECEIVE_BUFFER_SIZE = 1024;

			class BaseSocket {

			private:
				::SOCKET m_hSocket;
				char m_sReceiveMessageBuffer[MAX_RECEIVE_BUFFER_SIZE];

			protected:
				inline char* const GetReceiveBuffer() { return m_sReceiveMessageBuffer; }

			public:
				BaseSocket(const UTIL::SOCKET::EPROTOCOLTYPE protocolType);
				virtual ~BaseSocket();

			public:
				bool Bind(const FUNCTIONS::SOCKETADDRESS::SocketAddress& bindAddres);

				virtual bool SendCompletion(const uint16_t iSendBytes) = 0;

				inline ::SOCKET GetSocket() const { return m_hSocket; }

			};
		}
	}
}