#pragma once
#include <Network/Session/PacketSession/PacketSession.h>
#include <Network/Socket/TCP/TCPSocket.h>
#include <Network/Socket/UDP/UDPSocket.h>

namespace NETWORK {
	namespace SESSION {
		namespace NETWORKSESSION {
			class CNetworkSession;
		}
	}

	namespace UTIL {
		namespace NETWORKSESSION {
			namespace SERVERSESSION {
				namespace DETAIL {
					struct OVERLAPPED_EX;
				}
			}

			inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::CNetworkSession& Session);
		}
	}

	namespace SESSION {
		namespace NETWORKSESSION {
			class CNetworkSession {
				friend ::SOCKET UTIL::NETWORKSESSION::GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::CNetworkSession& Session);
			private:
				SESSION::PACKETSESSION::CPacketSession m_PacketSession;
				NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE m_ProtocolType;

			private:
				std::unique_ptr<SOCKET::TCPIP::CTCPIPSocket> m_TCPSocket;
				std::unique_ptr<SOCKET::UDPIP::CUDPIPSocket> m_UDPSocket;

			public:
				explicit CNetworkSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CNetworkSession();

			public:
				inline bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress) {
					return m_TCPSocket->Connect(ConnectAddress);
				}
				inline bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress, const size_t BackLogCount) {
					if (m_ProtocolType & NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP) {
						if (!m_TCPSocket->Bind(BindAddress) || !m_TCPSocket->Listen(BackLogCount)) { return false; }
					}
					if (m_ProtocolType & NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP) {
						if (!m_UDPSocket->Bind(BindAddress)) { return false; }
					}
					return true;
				}
				inline bool Initialize(CNetworkSession& ListenSession, UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& AcceptOverlapped) {
					return m_TCPSocket->Accept(*ListenSession.m_TCPSocket, AcceptOverlapped);
				}

			public:
				inline bool WriteIOCP(const char* const SendData, const size_t& DataLength, UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped) {
					return m_TCPSocket->Write(SendData, DataLength, &SendOverlapped.m_Overlapped);
				}
				inline bool WriteToIOCP() {
					//	return m_UDPSocket->WriteTo();
				}

			public:
				inline bool WriteEventSelect(const char* const SendData, const size_t& DataLength){
					return m_TCPSocket->Write(SendData, DataLength, nullptr);
				}
				inline bool WriteToEventSelect() {
					//	return m_UDPSocket->WriteTo();
				}

			public:
				inline bool ReadIOCP(UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& RecvOverlapped) {
					return m_TCPSocket->Read(nullptr, size_t(), &RecvOverlapped.m_Overlapped);
				}
				inline bool ReadEventSelect(char* const ReadBuffer, size_t& ReadedSize) {
					return m_TCPSocket->Read(ReadBuffer, std::move(ReadedSize), nullptr);
				}

			public:
				inline bool ReadFromIOCP() {
					return true;
				}
				inline bool ReadFromEventSelect() {
					return true;
				}

			};
		}
	}

	namespace UTIL {
		namespace NETWORKSESSION {
			inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::CNetworkSession& Session) {
				return ProtocolType == BASESOCKET::EPROTOCOLTYPE::EPT_TCP ? BASESOCKET::GetSocketValue(*Session.m_TCPSocket) : BASESOCKET::GetSocketValue(*Session.m_UDPSocket);
			}
		}
	}
}