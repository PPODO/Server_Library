#pragma once
#include <Network/Socket/Socket.h>
#include <memory>

namespace NETWORK {
	namespace UTIL {
		namespace SESSION {
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
				bool Listen(const int32_t& BackLogCount = SOMAXCONN);
				bool Connect(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress);
				bool Accept(const CTCPIPSocket& ListenSocket, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& AcceptOverlapped);

			public:
				bool Write(const char* const SendData, const size_t& DataLength, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped);
				bool Read(char* const ReadBuffer, size_t&& ReadedSize, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& RecvOverlapped);

			public:
				bool SocketRecycling(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& DisconnectOverlapped);

			};
		}
	}

	namespace UTIL {
		namespace TCPIP {
			inline bool Send(const ::SOCKET& Socket, const char* const SendBuffer, const size_t& SendBufferSize, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped);

			inline bool Receive(const ::SOCKET& Socket, char* const ReceiveBuffer, size_t& ReceiveBufferSize, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& RecvOverlapped);
		}
	}
}