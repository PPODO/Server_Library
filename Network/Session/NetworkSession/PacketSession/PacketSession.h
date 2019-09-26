#pragma once
#include <Network/Session/NetworkSession/NetworkSession.h>

namespace NETWORK {
	namespace SESSION {
		namespace PACKETSESSION {
			class CPacketSession;
		}
	}

	namespace UTIL {
		namespace PACKETSESSION {
			namespace DETAIL {
				inline ::SOCKET GetSocketValueFromPacketSession(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, SESSION::PACKETSESSION::CPacketSession& Session);
			}
		}
	}

	namespace SESSION {
		namespace PACKETSESSION {
			class CPacketSession {
				friend ::SOCKET UTIL::PACKETSESSION::DETAIL::GetSocketValueFromPacketSession(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, CPacketSession& Session);
			private:
				std::shared_ptr<NETWORKSESSION::CNetworkSession> m_NetworkSession;

			protected:
				inline std::shared_ptr<NETWORKSESSION::CNetworkSession> GetNetworkSession() const { return m_NetworkSession; }

			public:
				explicit CPacketSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CPacketSession();

			};
		}
	}
	
	namespace UTIL {
		namespace PACKETSESSION {
			namespace DETAIL {
				inline ::SOCKET GetSocketValueFromPacketSession(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, SESSION::PACKETSESSION::CPacketSession& Session) {
					return NETWORKSESSION::GetSocketValue(ProtocolType, Session.m_NetworkSession);
				}
			}

			inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, std::shared_ptr<SESSION::PACKETSESSION::CPacketSession> Session) {
				return DETAIL::GetSocketValueFromPacketSession(ProtocolType , *Session);
			}
			inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, SESSION::PACKETSESSION::CPacketSession& Session) {
				return DETAIL::GetSocketValueFromPacketSession(ProtocolType, Session);
			}
		}
	}
}