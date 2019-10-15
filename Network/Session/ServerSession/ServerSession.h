#pragma once
#include <Network/Socket/TCP/TCPSocket.h>
#include <Network/Socket/UDP/UDPSocket.h>

namespace NETWORK {
	namespace SESSION {
		namespace SERVERSESSION {

			class CServerSession {
			private:
				std::unique_ptr<SOCKET::TCPIP::CTCPIPSocket> m_TCPSocket;
				std::unique_ptr<SOCKET::UDPIP::CUDPIPSocket> m_UDPSocket;

			private:
				UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_AcceptOverlapped;
				UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_DisconnectOverlapped; 
				UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_ReceiveOverlapped;
				UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_SendOverlapped;

			public:
				explicit CServerSession(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CServerSession();

			public:
				bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress, const int32_t& BackLogCount = SOMAXCONN);
				bool Initialize(const CServerSession& ServerSession);
				bool SocketRecycle();

			public:
				inline bool Receive() {
					if (m_TCPSocket) {
						return m_TCPSocket->Read(nullptr, 0, m_ReceiveOverlapped);
					}
					return false;
				}

			public:
				bool Send() {

				}

			public:
				bool RegisterIOCompletionPort(const HANDLE& hIOCP);

			};

		}
	}
}