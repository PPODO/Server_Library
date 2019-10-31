#pragma once
#include <NetworkModel/NetworkModel/NetworkModel.h>

namespace NETWORKMODEL {
	namespace EVENTSELECT {
		class CEventSelect : public DETAIL::CNetworkModel {
		private:
			const int m_PacketProcessLoopCount;

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
			void EventSelectProcessorForTCP(const HANDLE& SelectEventHandle);
			void EventSelectProcessorForUDP(const HANDLE& SelectEventHandle);

		public:
			explicit CEventSelect(const int PacketProcessLoopCount, const DETAIL::PACKETPROCESSORLIST& ProcessorList);
			virtual ~CEventSelect() override;

		public:
			virtual bool Initialize(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& ServerAddress) override;
			virtual void Run() override;

		protected:
			virtual void Destroy() override;

		public:
			inline bool Send(const NETWORK::PACKET::PACKET_STRUCTURE& PacketStructure) {
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
			inline bool SendTo(NETWORK::PACKET::PACKET_STRUCTURE& PacketStructure) {
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

		};
	}

	namespace UTIL {
		namespace EVENTSELECT {
			inline bool Send(NETWORKMODEL::EVENTSELECT::CEventSelect& EventSelect, const char* const DataBuffer, const uint16_t& DataLength) {
				return EventSelect.Send(DataBuffer, DataLength);
			}
			inline bool Send(NETWORKMODEL::EVENTSELECT::CEventSelect& EventSelect, NETWORK::PACKET::PACKET_STRUCTURE& PacketStructure) {
				return EventSelect.Send(PacketStructure);
			}
			inline bool SendTo(NETWORKMODEL::EVENTSELECT::CEventSelect& EventSelect, const char* const DataBuffer, const uint16_t& DataLength) {
				return EventSelect.SendTo(DataBuffer, DataLength);
			}
			inline bool SendTo(NETWORKMODEL::EVENTSELECT::CEventSelect& EventSelect, const NETWORK::PACKET::PACKET_STRUCTURE& PacketStructure) {
				return EventSelect.SendTo(const_cast<NETWORK::PACKET::PACKET_STRUCTURE&>(PacketStructure));
			}
		}
	}
}

namespace NETWORK {
	namespace UTIL {
		namespace UDPIP {
			bool CheckAck(NETWORK::SOCKET::UDPIP::CUDPIPSocket* const UDPSocket, const FUNCTIONS::SOCKADDR::CSocketAddress& RemoteAddress, char* const ReceviedBuffer, uint16_t& ReceivedBytes, int16_t& UpdatedPacketNumber);
		}
	}
}