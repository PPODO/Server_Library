#pragma once
#include <Network/Socket/TCP/TCPSocket.h>
#include <Network/Socket/UDP/UDPSocket.h>

namespace NETWORK {
	namespace UTIL {
		namespace NETWORKSESSION {
			enum ESESSIONTYPE { EST_CLIENT, EST_SERVER };
		}
	}

	namespace SESSION {
		namespace NETWORKSESSION {
			// 파생 클래스는 CNetSession 클래스를 통해 '구현'되기에 private 상속을 사용.
			class CNetworkSession {
			private:
				NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE m_ProtocolType;

			private:
				std::shared_ptr<SOCKET::TCPIP::CTCPIPSocket> m_TCPSocket;
				std::shared_ptr<SOCKET::UDPIP::CUDPIPSocket> m_UDPSocket;

			private:
				NETWORK::UTIL::BASESOCKET::OVERLAPPED_EX m_SendOverlapped;
				NETWORK::UTIL::BASESOCKET::OVERLAPPED_EX m_RecvOverlapped;
				NETWORK::UTIL::BASESOCKET::OVERLAPPED_EX m_AcceptOverlapped;

			public:
				explicit CNetworkSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				~CNetworkSession();

			public:
				inline bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress) {
					return m_TCPSocket->Connect(ConnectAddress);
				}
				inline bool Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress, const size_t BackLogCount) {
					if (m_ProtocolType & NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP) {
						if (!m_TCPSocket->Bind(BindAddress) || !m_TCPSocket->Listen(BackLogCount)) { return false; }
					}
					if (m_ProtocolType & NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP) {
						if (!m_UDPSocket->Bind(BindAddress)) { return false; }
					}
					return true;
				}
				inline bool Initialize(std::shared_ptr<CNetworkSession> ListenSession) {
					return m_TCPSocket->Accept(ListenSession->m_TCPSocket, ListenSession->m_AcceptOverlapped);
				}

			public:
				inline bool Write(const UTIL::NETWORKSESSION::ESESSIONTYPE& SessionType, const char* const SendData, const size_t& DataLength) {
					if (SessionType == UTIL::NETWORKSESSION::EST_CLIENT) {
						return m_TCPSocket->Write(SendData, DataLength);
					}
					else {
						return m_TCPSocket->Write(SendData, DataLength, m_SendOverlapped);
					}
					return false;
				}
				inline bool WriteTo(const UTIL::NETWORKSESSION::ESESSIONTYPE& SessionType) {
					if (SessionType == UTIL::NETWORKSESSION::EST_CLIENT) {
					//	return m_UDPSocket->WriteTo();
					}
					else {
					//	return m_UDPSocket->WriteTo();
					}
					return false;
				}

			public:
				inline bool Read(const UTIL::NETWORKSESSION::ESESSIONTYPE& SessionType, char* const ReadBuffer, size_t& ReadedSize) {
					if (SessionType == UTIL::NETWORKSESSION::EST_CLIENT) {
						return m_TCPSocket->Read(ReadBuffer, ReadedSize);
					}
					else {
						return m_TCPSocket->Read(ReadBuffer, ReadedSize, m_RecvOverlapped);
					}
					return false;
				}
				inline bool ReadFrom(const UTIL::NETWORKSESSION::ESESSIONTYPE& SessionType) {
					if (SessionType == UTIL::NETWORKSESSION::EST_CLIENT) {
						return true;
					}
					else {
						return true;
					}
					return false;
				}

			};
		}
	}
}