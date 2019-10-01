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
				// Ŭ���̾�Ʈ������ ����� �� �ֽ��ϴ�.
				// ȥ���� ���� �� �����Ƿ� has a �� ������ ����?
				class CClientSession {
					friend ::SOCKET UTIL::NETWORKSESSION::CLIENTSESSION::GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::CLIENTSESSION::CClientSession& Session);

				private:
					NETWORKSESSION::CNetworkSession m_NetworkSession;

				public:
					explicit CClientSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
					virtual ~CClientSession();

				public:
					inline bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress) {
						return m_NetworkSession.Initialize(ConnectAddress);
					}

				public:
					inline bool Write(const char* const SendData, const size_t& DataLength) {
						return m_NetworkSession.WriteEventSelect(SendData, DataLength);
					}
					inline bool WriteTo() {
						return true;// GetNetworkSession()->WriteToEventSelect();
					}

				public:
					inline bool Read() {
						return true;// GetNetworkSession()->ReadEventSelect();
					}
					inline bool ReadFrom() {
						return m_NetworkSession.ReadFromEventSelect();
					}

				};
			}
		}
	}

	namespace UTIL {
		namespace NETWORKSESSION {
			namespace CLIENTSESSION {
				inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::CLIENTSESSION::CClientSession& Session) {
					return NETWORKSESSION::GetSocketValue(ProtocolType, Session.m_NetworkSession);
				}
			}
		}
	}
}