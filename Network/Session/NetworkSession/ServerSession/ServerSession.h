#pragma once
#include <Network/Session/NetworkSession/NetworkSession.h>

namespace NETWORK {
	namespace SESSION {
		namespace CLIENTSESSION {
			class CClientSession;
		}

		namespace SERVERSESSION {
			class CServerSession {
				friend CLIENTSESSION::CClientSession;
			private:
				std::shared_ptr<NETWORKSESSION::CNetworkSession> m_NetworkSession;

			public:
				explicit CServerSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CServerSession();

			public:
				inline bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) { return m_NetworkSession->Initialize(BindAddress, SOMAXCONN); }

			public:
				inline bool Write(const UTIL::NETWORKSESSION::ESESSIONTYPE& SessionType, const char* const SendData, const size_t& DataLength) {
					return m_NetworkSession->Write(UTIL::NETWORKSESSION::EST_SERVER, SendData, DataLength);
				}
				inline bool WriteTo(const UTIL::NETWORKSESSION::ESESSIONTYPE& SessionType) {

				}

			};
		}
	}
}