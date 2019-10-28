#pragma once
#include <Network/Socket/TCP/TCPSocket.h>
#include <Network/Socket/UDP/UDPSocket.h>
#include <vector>
#include <algorithm>

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
				UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_ReceiveFromOverlapped;

			private:
				FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_ConnectionListLock;
				std::vector<NETWORK::SOCKET::UDPIP::PEERINFO> m_ConnectedPeers;

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
						return m_TCPSocket->Read(m_ReceiveOverlapped);
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
				inline bool Send(const char* const SendData, const uint16_t& SendDataLength) {
					if (m_TCPSocket) {
						return m_TCPSocket->Write(SendData, SendDataLength, m_SendOverlapped);
					}
					return false;
				}

				inline bool Send(const PACKET::PACKET_STRUCTURE& PacketStructure) {
					if (m_TCPSocket) {
						return m_TCPSocket->Write(PacketStructure, m_SendOverlapped);
					}
					return false;
				}

				inline bool SendTo(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, NETWORK::PACKET::PACKET_STRUCTURE& SendPacketStructure) {
					if (auto PeerInfo = GetPeerInformation(SendAddress); m_UDPSocket) {
						SendPacketStructure.m_PacketInformation.m_PacketNumber = PeerInfo.m_LastPacketNumber;
						return m_UDPSocket->WriteToQueue(SendAddress, SendPacketStructure);
					}
					return false;
				}

				inline bool SendTo(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const char* const SendData, const uint16_t& SendDataLength) {
					if (m_UDPSocket) {
						return m_UDPSocket->WriteTo(SendAddress, SendData, SendDataLength);
					}
					return false;
				}

			public:
				bool SendCompletion(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);

			public:
				bool RegisterIOCompletionPort(const HANDLE& hIOCP);
				void UpdatePeerInformation(const FUNCTIONS::SOCKADDR::CSocketAddress& PeerAddress, const uint16_t& UpdatedPacketNumber);

			public:
				NETWORK::SOCKET::UDPIP::PEERINFO GetPeerInformation(const FUNCTIONS::SOCKADDR::CSocketAddress& PeerAddress) {
					FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_ConnectionListLock);

					if (const auto& Iterator = std::find_if(m_ConnectedPeers.begin(), m_ConnectedPeers.end(), [&PeerAddress](const NETWORK::SOCKET::UDPIP::PEERINFO& Address) -> bool { if (PeerAddress == Address.m_RemoteAddress) { return true; } return false; }); Iterator != m_ConnectedPeers.end()) {
						return *Iterator;
					}
					return NETWORK::SOCKET::UDPIP::PEERINFO();
				}

			};

		}
	}

	namespace UTIL {
		namespace UDPIP {
			bool CheckAck(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& Overlapped);
		}
	}
}