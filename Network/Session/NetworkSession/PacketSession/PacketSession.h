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
			inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::PACKETSESSION::CPacketSession& Session);
		}
	}

	namespace SESSION {
		namespace PACKETSESSION {
			static const size_t MAX_PACKET_BUFFER_SIZE = 1024;

			class CPacketSession {
				friend ::SOCKET UTIL::PACKETSESSION::GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const CPacketSession& Session);
			private:
				std::unique_ptr<NETWORKSESSION::CNetworkSession> m_NetworkSession;

			private:
				char m_PacketBuffer[MAX_PACKET_BUFFER_SIZE];

			public:
				explicit CPacketSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CPacketSession() = 0;

			public:
				inline NETWORKSESSION::CNetworkSession* const GetNetworkSession() const { return m_NetworkSession.get(); }

			};
		}
	}
	
	namespace UTIL {
		namespace PACKETSESSION {
			inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::PACKETSESSION::CPacketSession& Session) {
				return NETWORKSESSION::GetSocketValue(ProtocolType, *Session.m_NetworkSession);
			}
		}
	}
}