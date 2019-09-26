#pragma once
#include <Network/Session/NetworkSession/PacketSession/PacketSession.h>
#include <Network/Session/NetworkSession/PacketSession/ServerSession/ServerSession.h>

namespace NETWORK {
	namespace SESSION {
		namespace SERVERSESSION {
			class CServerSession;
		}

		namespace CLIENTSESSION {
			class CClientSession : public PACKETSESSION::CPacketSession {
			public:
				explicit CClientSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CClientSession();

			public:
				inline bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress) {
					return GetNetworkSession()->Initialize(ConnectAddress);
				}
				inline bool Initialize(SERVERSESSION::CServerSession& ListenSession) {
					return GetNetworkSession()->Initialize(ListenSession.GetNetworkSession());
				}
				inline bool Initialize(std::shared_ptr<SERVERSESSION::CServerSession> ListenSession) {
					return GetNetworkSession()->Initialize(ListenSession->GetNetworkSession());
				}

			public:
				inline bool Write(const UTIL::NETWORKSESSION::ESESSIONTYPE& SessionType, const char* const SendData, const size_t& DataLength) {
					return GetNetworkSession()->Write(UTIL::NETWORKSESSION::EST_CLIENT, SendData, DataLength);
				}
				inline bool WriteTo(const UTIL::NETWORKSESSION::ESESSIONTYPE& SessionType) {
					
				}

			public:
				inline bool Read() {
					return true;//CNetworkSession::Read(EST_CLIENT);
				}
				inline bool ReadFrom() {
					return GetNetworkSession()->ReadFrom(UTIL::NETWORKSESSION::EST_CLIENT);
				}

			};
		}
	}

	namespace UTIL {
		namespace CLIENTSESSION {


		}
	}
}