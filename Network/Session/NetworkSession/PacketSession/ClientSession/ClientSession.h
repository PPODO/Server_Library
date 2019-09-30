#pragma once
#include <Network/Session/NetworkSession/PacketSession/PacketSession.h>

namespace NETWORK {
	namespace SESSION {
		namespace CLIENTSESSION {
			class CClientSession;
		}
	}

	namespace UTIL {
		namespace CLIENTSESSION {
			inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::CLIENTSESSION::CClientSession& Session);
		}
	}

	namespace SESSION {
		namespace CLIENTSESSION {
			// Ŭ���̾�Ʈ������ ����� �� �ֽ��ϴ�.
			// ȥ���� ���� �� �����Ƿ� has a �� ������ ����?
			class CClientSession {
				friend ::SOCKET UTIL::CLIENTSESSION::GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::CLIENTSESSION::CClientSession& Session);

				class CClientSessionImplement : public PACKETSESSION::CPacketSession {
				public:
					explicit CClientSessionImplement(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : PACKETSESSION::CPacketSession(ProtocolType) {}
					virtual ~CClientSessionImplement() {}

				};

			private:
				CClientSessionImplement m_Session;

			public:
				explicit CClientSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CClientSession();

			public:
				inline bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress) {
					return m_Session.GetNetworkSession()->Initialize(ConnectAddress);
				}

			public:
				inline bool Write(const char* const SendData, const size_t& DataLength) {
					return m_Session.GetNetworkSession()->WriteEventSelect(SendData, DataLength);
				}
				inline bool WriteTo() {
					return true;// GetNetworkSession()->WriteToEventSelect();
				}

			public:
				inline bool Read() {
					return true;// GetNetworkSession()->ReadEventSelect();
				}
				inline bool ReadFrom() {
					return m_Session.GetNetworkSession()->ReadFromEventSelect();
				}

			};
		}
	}

	namespace UTIL {
		namespace CLIENTSESSION {
			inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::CLIENTSESSION::CClientSession& Session) {
				return PACKETSESSION::GetSocketValue(ProtocolType, Session.m_Session);
			}
		}
	}
}