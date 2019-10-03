#pragma once
#include <Network/Session/PacketSession/PacketSession.h>
#include <Network/Socket/Socket.h>
#include <Network/Socket/TCP/TCPSocket.h>
#include <Network/Socket/UDP/UDPSocket.h>

namespace NETWORK {
	namespace SESSION {
		namespace NETWORKSESSION {
			class CNetworkSession;
		}
	}

	namespace UTIL {
		namespace NETWORKSESSION {
			namespace SERVERSESSION {
				namespace DETAIL {
					struct OVERLAPPED_EX;
				}
			}

			inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::CNetworkSession& Session);
		}
	}

	namespace SESSION {
		namespace NETWORKSESSION {
			class CNetworkSession {
				friend ::SOCKET UTIL::NETWORKSESSION::GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::CNetworkSession& Session);
			private:
				class CPacketSessionImple : public PACKETSESSION::CPacketSession {
				private:
					FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection m_BufferLock;

				private:
					char m_Buffer[SOCKET::BASESOCKET::MAX_RECEIVE_BUFFER_SIZE];

				private:
					uint16_t m_CurrentReadedBytes;

				public:
					explicit CPacketSessionImple() : m_CurrentReadedBytes(0) { ZeroMemory(m_Buffer, SOCKET::BASESOCKET::MAX_RECEIVE_BUFFER_SIZE); };
					virtual ~CPacketSessionImple() {};

				public:
					const FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const ReadSomething(const std::shared_ptr<SOCKET::BASESOCKET::CBaseSocket>& Socket, const DWORD& RecvBytes) {
						try {
							FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_BufferLock);

							if (Socket) {
								Socket->CopyReceiveBuffer(m_Buffer + m_CurrentReadedBytes, RecvBytes);
								m_CurrentReadedBytes += RecvBytes;

								return ReceivedDataDeSerialization(m_Buffer, m_CurrentReadedBytes);
							}
						}
						catch (const std::bad_cast& Exception) {
							FUNCTIONS::LOG::CLog::WriteLog(Exception.what());
							return nullptr;
						}
					}

				};

			private:
				CPacketSessionImple m_PacketSession;
				NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE m_ProtocolType;

			private:
				std::shared_ptr<SOCKET::TCPIP::CTCPIPSocket> m_TCPSocket;
				std::shared_ptr<SOCKET::UDPIP::CUDPIPSocket> m_UDPSocket;

			public:
				explicit CNetworkSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType);
				virtual ~CNetworkSession() = 0;

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
				inline bool Initialize(CNetworkSession& ListenSession, UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& AcceptOverlapped) {
					return m_TCPSocket->Accept(*ListenSession.m_TCPSocket, AcceptOverlapped);
				}
				
			public:
				inline bool WriteIOCP(const char* const SendData, const size_t& DataLength, UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped) {
					return m_TCPSocket->Write(SendData, DataLength, &SendOverlapped.m_Overlapped);
				}
				inline bool WriteToIOCP() {
					//	return m_UDPSocket->WriteTo();
				}

			public:
				inline bool WriteEventSelect(const char* const SendData, const size_t& DataLength){
					return m_TCPSocket->Write(SendData, DataLength, nullptr);
				}
				inline bool WriteToEventSelect() {
					//	return m_UDPSocket->WriteTo();
				}

			public:
				inline bool ReadIOCP(UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& RecvOverlapped) {
					return m_TCPSocket->Read(nullptr, size_t(), &RecvOverlapped.m_Overlapped);
				}
				inline bool ReadEventSelect(char* const ReadBuffer, size_t& ReadedSize) {
					return m_TCPSocket->Read(ReadBuffer, std::move(ReadedSize), nullptr);
				}

			public:
				inline bool ReadFromIOCP(UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& RecvFromOverlapped) {
					return true;
				}
				inline bool ReadFromEventSelect() {
					return true;
				}

			public:
				inline const FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const GetReceivedPacket(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const DWORD& RecvBytes) {
					if (ProtocolType & UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP) {
						return m_PacketSession.ReadSomething(m_TCPSocket, RecvBytes);
					}
					return m_PacketSession.ReadSomething(m_UDPSocket, RecvBytes);
				}


			};
		}
	}

	namespace UTIL {
		namespace NETWORKSESSION {
			inline ::SOCKET GetSocketValue(const BASESOCKET::EPROTOCOLTYPE& ProtocolType, const SESSION::NETWORKSESSION::CNetworkSession& Session) {
				return ProtocolType == BASESOCKET::EPROTOCOLTYPE::EPT_TCP ? BASESOCKET::GetSocketValue(*Session.m_TCPSocket) : BASESOCKET::GetSocketValue(*Session.m_UDPSocket);
			}
		}
	}
}