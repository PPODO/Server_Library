#pragma once
#include <Network/Session/NetworkSession/PacketSession/PacketSession.h>

namespace NETWORK {
	namespace SESSION {
		namespace CLIENTSESSION {
			class CClientSession;
		}

		namespace SERVERSESSION {
			class CServerSession : public PACKETSESSION::CPacketSession {
				friend CLIENTSESSION::CClientSession;
			public:
				explicit CServerSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CServerSession();

			public:
				inline bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) { 
					return GetNetworkSession()->Initialize(BindAddress, SOMAXCONN); 
				}

			public:
				inline bool Write(const UTIL::NETWORKSESSION::ESESSIONTYPE& SessionType, const char* const SendData, const size_t& DataLength) {
					return GetNetworkSession()->Write(UTIL::NETWORKSESSION::EST_SERVER, SendData, DataLength);
				}
				inline bool WriteTo(const UTIL::NETWORKSESSION::ESESSIONTYPE& SessionType) {

				}

			};
		}
	}
}