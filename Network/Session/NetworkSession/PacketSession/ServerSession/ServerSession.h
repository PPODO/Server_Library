#pragma once
#include <Network/Session/NetworkSession/PacketSession/PacketSession.h>

namespace NETWORK {
	namespace SESSION {
		namespace SERVERSESSION {
			class CServerSession;
		}
	}

	namespace UTIL {
		namespace SERVERSESSION {
			inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::SERVERSESSION::CServerSession& Session);
		}
	}

	namespace SESSION {
		namespace SERVERSESSION {
			// 서버에서만 사용할 수 있습니다.
			class CServerSession {
				friend ::SOCKET UTIL::SERVERSESSION::GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::SERVERSESSION::CServerSession& Session);

				class CServerSessionImplement : public PACKETSESSION::CPacketSession {
				public:
					explicit CServerSessionImplement(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : PACKETSESSION::CPacketSession(ProtocolType) {}
					virtual ~CServerSessionImplement() {}

				};

			private:
				CServerSessionImplement m_Session;

			private:
				NETWORK::UTIL::BASESOCKET::OVERLAPPED_EX m_SendOverlapped;
				NETWORK::UTIL::BASESOCKET::OVERLAPPED_EX m_RecvOverlapped;
				NETWORK::UTIL::BASESOCKET::OVERLAPPED_EX m_AcceptOverlapped;
				NETWORK::UTIL::BASESOCKET::OVERLAPPED_EX m_DisconnectOverlapped;

			public:
				explicit CServerSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CServerSession();

			public:
				inline bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) { 
					return m_Session.GetNetworkSession()->Initialize(BindAddress, SOMAXCONN); 
				}
				inline bool Initialize(SERVERSESSION::CServerSession& ListenSession) {
					return m_Session.GetNetworkSession()->Initialize(*ListenSession.m_Session.GetNetworkSession(), m_AcceptOverlapped);
				}

			public:
				inline bool Write(const char* const SendData, const size_t& DataLength) {
					return m_Session.GetNetworkSession()->WriteIOCP(SendData, DataLength, m_SendOverlapped);
				}
				inline bool WriteTo() {
					return true;
				}

			public:
				inline bool Read() {
					return m_Session.GetNetworkSession()->ReadIOCP(m_RecvOverlapped);
				}
				inline bool ReadFrom() {
					return m_Session.GetNetworkSession()->ReadFromIOCP();
				}

			public:
				inline bool Reload() {
					return UTIL::TCPIP::ReUseSocket(UTIL::NETWORKSESSION::GetSocketValue(UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP, *m_Session.GetNetworkSession()), m_DisconnectOverlapped);
				}

			};
		}
	}

	namespace UTIL {
		namespace SERVERSESSION {
			inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::SERVERSESSION::CServerSession& Session) {
				return PACKETSESSION::GetSocketValue(ProtocolType, Session.m_Session);
			}
		}
	}
}