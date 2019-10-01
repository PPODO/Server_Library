#pragma once
#include <Network/Session/NetworkSession/NetworkSession.h>

namespace NETWORK {
	namespace SESSION {
		namespace NETWORKSESSION {
			namespace SERVERSESSION {
				class CServerSession;
			}
		}
	}

	namespace UTIL {
		namespace NETWORKSESSION {
			namespace SERVERSESSION {
				inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::SERVERSESSION::CServerSession& Session);
			}
		}
	}

	namespace SESSION {
		namespace NETWORKSESSION {
			namespace SERVERSESSION {
				// 서버에서만 사용할 수 있습니다.
				class CServerSession {
					friend ::SOCKET UTIL::NETWORKSESSION::SERVERSESSION::GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::SERVERSESSION::CServerSession& Session);

				private:
					NETWORKSESSION::CNetworkSession m_NetworkSession;

				private:
					NETWORK::UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_SendOverlapped;
					NETWORK::UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_RecvOverlapped;
					NETWORK::UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_AcceptOverlapped;
					NETWORK::UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX m_DisconnectOverlapped;

				public:
					explicit CServerSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
					virtual ~CServerSession();

				public:
					inline bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) {
						return m_NetworkSession.Initialize(BindAddress, SOMAXCONN);
					}
					inline bool Initialize(SERVERSESSION::CServerSession& ListenSession) {
						return m_NetworkSession.Initialize(ListenSession.m_NetworkSession, m_AcceptOverlapped);
					}

				public:
					inline bool Write(const char* const SendData, const size_t& DataLength) {
						return m_NetworkSession.WriteIOCP(SendData, DataLength, m_SendOverlapped);
					}
					inline bool WriteTo() {
						return true;
					}

				public:
					inline bool Read() {
						return m_NetworkSession.ReadIOCP(m_RecvOverlapped);
					}
					inline bool ReadFrom() {
						return m_NetworkSession.ReadFromIOCP();
					}

				public:
					bool SocketRecycling() {
						return UTIL::TCPIP::SocketRecycling(UTIL::NETWORKSESSION::SERVERSESSION::GetSocketValue(UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP, *this), &m_DisconnectOverlapped.m_Overlapped);
					}

				};
			}
		}
	}

	namespace UTIL {
		namespace NETWORKSESSION {
			namespace SERVERSESSION {
				inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::SERVERSESSION::CServerSession& Session) {
					return NETWORKSESSION::GetSocketValue(ProtocolType, Session.m_NetworkSession);
				}
			}
		}
	}
}