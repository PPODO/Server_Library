#pragma once
#include <Network/Socket/Socket.h>
#include <Functions/Functions/Log/Log.h>
#include <memory>

namespace NETWORK {
	namespace UTIL {
		namespace NETWORKSESSION {
			namespace SERVERSESSION {
				namespace DETAIL {
					struct OVERLAPPED_EX;
				}
			}
		}
	}

	namespace SOCKET {
		namespace TCPIP {
			class CTCPIPSocket : public BASESOCKET::CBaseSocket {
			public:
				explicit CTCPIPSocket();
				virtual ~CTCPIPSocket() override;

			public:
				bool Listen(const size_t BackLogCount = SOMAXCONN);
				bool Connect(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress);
				bool Accept(const CTCPIPSocket& ListenSocket, UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& AcceptOverlapped);

			public:
				bool Write(const char* const SendData, const size_t& DataLength, WSAOVERLAPPED* const SendOverlapped);
				bool Read(char* const ReadBuffer, size_t&& ReadedSize, WSAOVERLAPPED* const RecvOverlapped);

			};
		}
	}

	namespace UTIL {
		namespace TCPIP {
			inline bool Send(const ::SOCKET& Socket, const char* const SendBuffer, const size_t& SendBufferSize, WSAOVERLAPPED* const SendOverlapped);

			inline bool Receive(const ::SOCKET& Socket, char* const ReceiveBuffer, size_t& ReceiveBufferSize, WSAOVERLAPPED* const RecvOverlapped);

			bool SocketRecycling(const ::SOCKET& Socket, WSAOVERLAPPED* const DisconnectOverlapped);
		}
	}
}