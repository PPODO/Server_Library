#pragma once
#include <Network/Socket/TCP/TCPSocket.hpp>
#include <Network/Socket/UDP/UDPSocket.hpp>
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
				UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_ReceiveFromOverlapped;

			private:
				FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_SendOverlappedLock;
				UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_SendOverlapped;

			public:
				explicit CServerSession(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				explicit CServerSession(CServerSession&& rvalue);
				virtual ~CServerSession();

			public:
				bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress, const int32_t& BackLogCount = SOMAXCONN);
				bool Initialize(const CServerSession& ServerSession);

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
						FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_SendOverlappedLock);

						return m_TCPSocket->Write(SendData, SendDataLength, m_SendOverlapped);
					}
					return false;
				}
				inline bool Send(const PACKET::PACKET_STRUCTURE& PacketStructure) {
					if (m_TCPSocket) {
						FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(&m_SendOverlappedLock);

						return m_TCPSocket->Write(PacketStructure, m_SendOverlapped);
					}
					return false;
				}
				inline bool SendTo(NETWORK::SOCKET::UDPIP::PEERINFO& PeerInformation, NETWORK::PACKET::PACKET_STRUCTURE& SendPacketStructure) {
					if (m_UDPSocket) {
						SendPacketStructure.m_PacketInformation.m_PacketNumber = PeerInformation.m_LastPacketNumber;
						return m_UDPSocket->WriteToReliable(PeerInformation.m_RemoteAddress, SendPacketStructure);
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
				inline bool SocketRecycle() {
					if (m_TCPSocket) {
						return m_TCPSocket->SocketRecycling(m_DisconnectOverlapped);
					}
					return false;
				}

			public:
				inline bool SendCompletion(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const uint16_t& SendBytes) {
					if (ProtocolType == UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP) {
						return m_TCPSocket->SendCompletion(SendBytes);
					}
					else if (ProtocolType == UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP) {
						return m_UDPSocket->SendCompletion(SendBytes);
					}
					return false;
				}

			public:
				bool RegisterIOCompletionPort(const HANDLE& hIOCP);

			};

		}
	}

	namespace UTIL {
		namespace UDPIP {
			bool CheckAck(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& Overlapped);
		}
	}
}