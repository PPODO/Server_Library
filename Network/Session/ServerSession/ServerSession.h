#pragma once
#include <Network/Socket/TCP/TCPSocket.h>
#include <Network/Socket/UDP/UDPSocket.h>
#include <algorithm>

namespace NETWORK {
	namespace SESSION {
		namespace SERVERSESSION {

			class CServerSession {
			private:
				std::unique_ptr<SOCKET::TCPIP::CTCPIPSocket> m_TCPSocket;
				std::unique_ptr<SOCKET::UDPIP::CUDPIPSocket> m_UDPSocket;

			private:
				FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_ConnectionListLock;
				std::vector<FUNCTIONS::SOCKADDR::CSocketAddress> m_ConnectedPeers;

			private:
				UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_AcceptOverlapped;
				UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_DisconnectOverlapped; 
				UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_ReceiveOverlapped;
				UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_SendOverlapped;
				UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_ReceiveFromOverlapped;

			private:
				inline void RegisterNewPeer(const FUNCTIONS::SOCKADDR::CSocketAddress& NewPeerAddress) {
					FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_ConnectionListLock);

					if (auto Iterator = std::find_if(m_ConnectedPeers.cbegin(), m_ConnectedPeers.cend(), [&NewPeerAddress](const FUNCTIONS::SOCKADDR::CSocketAddress& PeerAddress) -> bool { if (PeerAddress == NewPeerAddress) { return true; } return false; }); Iterator == m_ConnectedPeers.cend()) {
						m_ConnectedPeers.emplace_back(NewPeerAddress);
						FUNCTIONS::LOG::CLog::WriteLog(L"Register New Peer!");
					}
				}

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

				inline bool ReceiveFrom() {
					if (m_UDPSocket) {
						return m_UDPSocket->ReadFrom(m_ReceiveFromOverlapped);
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