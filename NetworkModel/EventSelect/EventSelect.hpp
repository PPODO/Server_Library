#pragma once
#include "../BaseModel/BaseModel.hpp"
#include "../../Network/NetworkProtocol/TCPSocket.hpp"
#include "../../Network/NetworkProtocol/UDPSocket.hpp"

using namespace SERVER::NETWORKMODEL::BASEMODEL;
using namespace SERVER::NETWORK::USER_SESSION;

namespace SERVER {
	namespace NETWORKMODEL {
		namespace EVENTSELECT {

			class EventSelect : public BaseNetworkModel {
				class EventSelectClient : public User {
				public:
					EventSelectClient(const EPROTOCOLTYPE protocolType) : User(protocolType) {}

				public:
					inline bool Send(const NETWORK::PACKET::PACKET_STRUCT& packet) {
						if (m_pTCPSocekt) {
							return m_pTCPSocekt->Write(packet);
						}
						return false;
					}

					inline bool Send(const char* const DataBuffer, const uint16_t& DataLength) {
						if (m_pTCPSocekt) {
							return m_pTCPSocekt->Write(DataBuffer, DataLength);
						}
						return false;
					}

					inline bool SendTo(const FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress, NETWORK::PACKET::PACKET_STRUCT& packet, int16_t& iPacketNumber) {
						if (m_pUDPSocket) {
							packet.m_packetInfo.m_iPacketNumber = iPacketNumber;
							InterlockedIncrement16(&iPacketNumber);
							return m_pUDPSocket->WriteToReliable(serverAddress, packet);
						}
						return false;
					}

					inline bool SendToUnReliable(const FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress, const NETWORK::PACKET::PACKET_STRUCT& packet) {
						if (m_pUDPSocket)
							return m_pUDPSocket->WriteToUnReliable(serverAddress, packet);
						return false;
					}

					inline bool SendTo(const FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress, const char* const DataBuffer, const uint16_t& DataLength) {
						if (m_pUDPSocket) {
							return m_pUDPSocket->WriteTo(serverAddress, DataBuffer, DataLength);
						}
						return false;
					}

					inline SOCKET GetTCPSocketHandle() const {
						if (m_pTCPSocekt)
							return m_pTCPSocekt->GetSocket();

						return INVALID_SOCKET;
					}
					inline SOCKET GetUDPSocketHandle() const {
						if (m_pUDPSocket)
							return m_pUDPSocket->GetSocket();

						return INVALID_SOCKET;
					}

					inline UDPIPSocket* GetUDPInstance() const { return m_pUDPSocket.get(); }
				};

			private:
				WSAEVENT m_hEventSelect[2];
				WSAEVENT m_hStopHandle;

				std::unique_ptr<EventSelectClient> m_client;
				FUNCTIONS::SOCKETADDRESS::SocketAddress m_serverAddress;

				int16_t m_iThreadRunState;
				std::thread m_threadForTCPSelectProcessor;
				std::thread m_threadForUDPSelectProcessor;

				int16_t m_iNextSendPacketNumber;

			private:
				bool InitializeEventHandle();

				void EventSelectProcessorForTCP(const WSAEVENT& hEventHandle);
				void EventSelectProcessorForUDP(const WSAEVENT& hEventHandle);

			public:
				EventSelect(const int iPacketProcessorLoopCount, const PACKETPROCESSOR& packetProcessorMap);
				virtual ~EventSelect() override;

				virtual bool Initialize(const EPROTOCOLTYPE protocolType, FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress) override;
				virtual void Run() override;
				virtual void Destroy() override;

				inline bool Send(const NETWORK::PACKET::PACKET_STRUCT& packet) {
					if (m_client)
						return m_client->Send(packet);
					return false;
				}

				inline bool SendTo(const bool bReliable, const NETWORK::PACKET::PACKET_STRUCT& packet) {
					if (m_client) {
						if (bReliable)
							return m_client->SendTo(m_serverAddress, const_cast<NETWORK::PACKET::PACKET_STRUCT&>(packet), m_iNextSendPacketNumber);
						else
							return m_client->SendToUnReliable(m_serverAddress, packet);
					}
					return false;
				}

			};
		}
	}
}