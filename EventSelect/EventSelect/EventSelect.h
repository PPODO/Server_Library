#pragma once
#include <Network/Socket/TCP/TCPSocket.h>
#include <Network/Socket/UDP/UDPSocket.h>
#include <thread>
#include <array>

namespace NETWORK {
	namespace NETWORKMODEL {
		namespace EVENTSELECT {
			class CEventSelect {
			private:
				WSADATA m_WinSockData;
				
			private:
				HANDLE m_hTCPSelectEvent;
				HANDLE m_hUDPSelectEvent;
				HANDLE m_hStopEvent;

			private:
				int16_t m_ThreadRunState;
				std::vector<std::thread> m_EventSelectThread;

			private:
				FUNCTIONS::SOCKADDR::CSocketAddress m_ServerAddress;
				std::unique_ptr<NETWORK::SOCKET::TCPIP::CTCPIPSocket> m_TCPIPSocket;
				std::unique_ptr<NETWORK::SOCKET::UDPIP::CUDPIPSocket> m_UDPIPSocket;

			private:
				void EventSelectProcessorForTCP(const HANDLE& SelectEventHandle);
				void EventSelectProcessorForUDP(const HANDLE& SelectEventHandle);

			private:
				void PacketForwardingLoop(char* const ReceivedBuffer, int16_t& ReceivedBytes, int16_t& LastReceivedPacketNumber);

			public:
				explicit CEventSelect(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& ServerAddress);
				virtual ~CEventSelect();

			public:
				bool Initialize();

			public:
				inline bool Send(const PACKET::PACKET_STRUCTURE& PacketStructure) {
					if (m_TCPIPSocket) {
						return m_TCPIPSocket->Write(PacketStructure);
					}
					return false;
				}
				inline bool Send(const char* const DataBuffer, const uint16_t& DataLength) {
					if (m_TCPIPSocket) {
						return m_TCPIPSocket->Write(DataBuffer, DataLength);
					}
					return false;
				}
				inline bool SendTo(const PACKET::PACKET_STRUCTURE& PacketStructure) {
					if (m_UDPIPSocket) {
						return m_UDPIPSocket->WriteTo(m_ServerAddress, PacketStructure);
					}
					return false;
				}
				inline bool SendTo(const char* const DataBuffer, const uint16_t& DataLength) {
					if (m_UDPIPSocket) {
						return m_UDPIPSocket->WriteTo(m_ServerAddress, DataBuffer, DataLength);
					}
					return false;
				}

			};
		}
	}

	namespace UTIL {
		namespace NETWORKMODEL {
			namespace EVENTSELECT {
				inline bool Send(NETWORK::NETWORKMODEL::EVENTSELECT::CEventSelect& EventSelect, const char* const DataBuffer, const uint16_t& DataLength) {
					return EventSelect.Send(DataBuffer, DataLength);
				}
				inline bool Send(NETWORK::NETWORKMODEL::EVENTSELECT::CEventSelect& EventSelect, NETWORK::PACKET::PACKET_STRUCTURE& PacketStructure) {
					return EventSelect.Send(PacketStructure);
				}
				inline bool SendTo(NETWORK::NETWORKMODEL::EVENTSELECT::CEventSelect& EventSelect, const char* const DataBuffer, const uint16_t& DataLength) {
					return EventSelect.SendTo(DataBuffer, DataLength);
				}
				inline bool SendTo(NETWORK::NETWORKMODEL::EVENTSELECT::CEventSelect& EventSelect, const NETWORK::PACKET::PACKET_STRUCTURE& PacketStructure) {
					return EventSelect.SendTo(PacketStructure);
				}

			}
		}
	}
}