#pragma once
#include <Network/Session/NetworkSession/NetworkSession.h>

namespace NETWORK {
	namespace SESSION {
		namespace NETWORKSESSION {
			namespace CLIENTSESSION {
				class CClientSession;
			}
		}
	}

	namespace UTIL {
		namespace NETWORKSESSION {
			namespace CLIENTSESSION {
				inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::CLIENTSESSION::CClientSession& Session);
			}
		}
	}

	namespace SESSION {
		namespace NETWORKSESSION {
			namespace CLIENTSESSION {
				class CClientSession {
					friend ::SOCKET UTIL::NETWORKSESSION::CLIENTSESSION::GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::CLIENTSESSION::CClientSession& Session);

				private:
					class CClientSessionImple : public NETWORKSESSION::CNetworkSession {
					public:
						CClientSessionImple(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : NETWORKSESSION::CNetworkSession(ProtocolType) {}
						virtual ~CClientSessionImple() {}
					};

				private:
					CClientSessionImple m_Session;

				public:
					explicit CClientSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
					virtual ~CClientSession();

				public:
					inline bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress) {
						return m_Session.Initialize(ConnectAddress);
					}

				public:
					inline bool Write(const char* const SendData, const size_t& DataLength) {
						return m_Session.WriteEventSelect(SendData, DataLength);
					}
					inline bool WriteTo() {
						return true;// GetNetworkSession()->WriteToEventSelect();
					}

				public:
					inline bool Read() {
						return true;// GetNetworkSession()->ReadEventSelect();
					}
					inline bool ReadFrom() {
						return m_Session.ReadFromEventSelect();
					}

				};
			}
		}
	}

	namespace UTIL {
		namespace NETWORKSESSION {
			namespace CLIENTSESSION {
				inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::CLIENTSESSION::CClientSession& Session) {
					return NETWORKSESSION::GetSocketValue(ProtocolType, Session.m_Session);
				}
			}
		}
	}
}