#pragma once
#include <Network/Session/NetworkSession/NetworkSession.h>
#include <Network/Session/NetworkSession/ServerSession/ServerSession.h>

namespace NETWORK {
	namespace SESSION {
		namespace SERVERSESSION {
			class CServerSession;
		}

		namespace CLIENTSESSION {
			class CClientSession {
			private:
				std::shared_ptr<NETWORKSESSION::CNetworkSession> m_NetworkSession;

			public:
				explicit CClientSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CClientSession();

			public:
				inline bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress) {
					return m_NetworkSession->Initialize(ConnectAddress);
				}
				inline bool Initialize(SERVERSESSION::CServerSession& ListenSession) {
					return m_NetworkSession->Initialize(ListenSession.m_NetworkSession);
				}
				inline bool Initialize(std::shared_ptr<SERVERSESSION::CServerSession> ListenSession) {
					return m_NetworkSession->Initialize(ListenSession->m_NetworkSession);
				}

			public:
				inline bool Write(const UTIL::NETWORKSESSION::ESESSIONTYPE& SessionType, const char* const SendData, const size_t& DataLength) {
					return m_NetworkSession->Write(UTIL::NETWORKSESSION::EST_CLIENT, SendData, DataLength);
				}
				inline bool WriteTo(const UTIL::NETWORKSESSION::ESESSIONTYPE& SessionType) {
					
				}

			public:
				inline bool Read() {
					return true;//CNetworkSession::Read(EST_CLIENT);
				}
				inline bool ReadFrom() {
					return m_NetworkSession->ReadFrom(UTIL::NETWORKSESSION::EST_CLIENT);
				}

			};
		}
	}

	namespace UTIL {
		namespace CLIENTSESSION {


		}
	}
}