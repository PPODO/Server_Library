#pragma once
#include <Functions/SocketAddress/SocketAddress.hpp>
#include <memory>
#include <vector>

namespace SERVER {
	namespace NETWORK {
		namespace PROTOCOL {
			namespace UTIL {
				namespace BSD_SOCKET {
					enum class EPROTOCOLTYPE {
						EPT_TCP = (1 << 0),
						EPT_UDP = (1 << 1),
						EPT_BOTH = (EPT_TCP | EPT_UDP)
					};

					bool operator&(const EPROTOCOLTYPE lhs, const EPROTOCOLTYPE rhs) {
						return (static_cast<int>(lhs) & static_cast<int>(rhs));
					}

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

					static int GetWSAErrorResult(const std::vector<int>& iErrorCode) {
						int iWSAErrorResult = WSAGetLastError();

						for (auto& iterator : iErrorCode)
							if (iWSAErrorResult == iterator) return 0;

						return iWSAErrorResult;
					}
				}

			}

			namespace BSD_SOCKET {
				static const size_t MAX_BUFFER_SIZE = 1024;

				class BaseSocket {
				private:
					::SOCKET m_hSocket;
					char m_sReceiveMessageBuffer[MAX_BUFFER_SIZE];

				protected:
					char m_sSendMessageBuffer[MAX_BUFFER_SIZE];

				protected:
					inline char* const GetReceiveBuffer() { return m_sReceiveMessageBuffer; }

				public:
					BaseSocket(const UTIL::BSD_SOCKET::EPROTOCOLTYPE protocolType);
					virtual ~BaseSocket();

				public:
					bool Bind(const FUNCTIONS::SOCKETADDRESS::SocketAddress& bindAddres);

					virtual bool SendCompletion(const uint16_t iSendBytes) = 0;

					inline ::SOCKET GetSocket() const { return m_hSocket; }

				};
			}
		}
	}
}