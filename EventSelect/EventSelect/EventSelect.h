#pragma once
#include <Network/Socket/TCP/TCPSocket.h>
#include <Network/Socket/UDP/UDPSocket.h>
#include <thread>
#include <vector>
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
				// For UDP
				int16_t m_NextSendPacketNumber;

			private:
				FUNCTIONS::SOCKADDR::CSocketAddress m_ServerAddress;
				std::unique_ptr<NETWORK::SOCKET::TCPIP::CTCPIPSocket> m_TCPIPSocket;
				std::unique_ptr<NETWORK::SOCKET::UDPIP::CUDPIPSocket> m_UDPIPSocket;

			private:
				FUNCTIONS::CIRCULARQUEUE::CCircularQueue<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData*> m_PacketQueue;

			private:
				void EventSelectProcessorForTCP(const HANDLE& SelectEventHandle);
				void EventSelectProcessorForUDP(const HANDLE& SelectEventHandle);

			private:
				void PacketForwardingLoop(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, char* const ReceivedBuffer, int16_t& ReceivedBytes, int16_t& LastReceivedPacketNumber);

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
				inline bool SendTo(PACKET::PACKET_STRUCTURE& PacketStructure) {
					if (m_UDPIPSocket) {
						PacketStructure.m_PacketInformation.m_PacketNumber = m_NextSendPacketNumber++;
						return m_UDPIPSocket->WriteToQueue(m_ServerAddress, PacketStructure);
					}
					return false;
				}
				inline bool SendTo(const char* const DataBuffer, const uint16_t& DataLength) {
					if (m_UDPIPSocket) {
						return m_UDPIPSocket->WriteTo(m_ServerAddress, DataBuffer, DataLength);
					}
					return false;
				}

			public:
				// Event Select는 메인스레드에서 루프걸고 패킷을 처리해서는 안되기 때문에 따로 데이터를 빼와서 처리.

				inline bool GetPacketDataFromQueue(FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* Data) {
					return m_PacketQueue.Pop(Data);
				}
				inline bool GetPacketDataFromQueue(std::unique_ptr<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData>& Data) {
					if (FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* PacketData; m_PacketQueue.Pop(PacketData)) {
						Data = std::unique_ptr<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData>(PacketData);
						return true;
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
					return EventSelect.SendTo(const_cast<NETWORK::PACKET::PACKET_STRUCTURE&>(PacketStructure));
				}

			}
		}

		namespace UDPIP {
			bool CheckAck(NETWORK::SOCKET::UDPIP::CUDPIPSocket* const UDPSocket, const FUNCTIONS::SOCKADDR::CSocketAddress& RemoteAddress, char* const ReceviedBuffer, uint16_t& ReceivedBytes, int16_t& UpdatedPacketNumber);
		}
	}
}