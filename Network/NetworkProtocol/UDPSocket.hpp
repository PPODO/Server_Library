#pragma once
#include "Socket/Socket.hpp"
#include "../Packet/BasePacket.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>

using namespace SERVER::FUNCTIONS::SOCKETADDRESS;

namespace SERVER {
	namespace NETWORK {
		namespace USER_SESSION {
			namespace USER_SERVER {
				struct OVERLAPPED_EX;
			}
		}

		namespace PROTOCOL {
			namespace UTIL {
				namespace UDP {
					bool SendTo(const ::SOCKET hSocket, const SocketAddress& sendAddress, const char* const sSendBuffer, const uint16_t iDataLength);
					bool ReceiveFrom(const ::SOCKET hSocket, char* const sReceiveBuffer, uint16_t& iReceiveBytes, SERVER::NETWORK::USER_SESSION::USER_SERVER::OVERLAPPED_EX& receiveOverlapped);

					bool CheckAck(USER_SESSION::USER_SERVER::OVERLAPPED_EX& overlapped);
				}
			}

			namespace UDP {
				namespace RELIABLE {
					struct ReliableData;
					class ReliableUDP;
				}

				struct PeerInfo {
				public:
					FUNCTIONS::SOCKETADDRESS::SocketAddress m_remoteAddress;
					uint16_t m_iLastPacketNumber;

				public:
					PeerInfo() : m_iLastPacketNumber(0) {}
					PeerInfo(const FUNCTIONS::SOCKETADDRESS::SocketAddress& remoteAddress, const uint16_t iLastPacketNumber) : m_remoteAddress(remoteAddress), m_iLastPacketNumber(iLastPacketNumber) {}
					
					bool operator==(const FUNCTIONS::SOCKETADDRESS::SocketAddress& address) const {
						if (m_remoteAddress == address) return true;
						return false;
					}

				};

				class UDPIPSocket : public PROTOCOL::BSD_SOCKET::BaseSocket {
				private:
					RELIABLE::ReliableUDP m_reliableProcessor;

				private:
					void SetAckNumberToBuffer(const NETWORK::PACKET::PACKET_STRUCT& sendPacketStructure);

				public:
					UDPIPSocket();
					virtual ~UDPIPSocket() override;

				public:
					bool WriteToReliable(const SocketAddress& sendAddress, const NETWORK::PACKET::PACKET_STRUCT& sendPacketStructure);
					bool WriteTo(const SocketAddress& sendAddress, const char* const sSendData, const uint16_t iDataLength);
					bool WriteTo(const FUNCTIONS::SOCKETADDRESS::SocketAddress& sendAddress, const NETWORK::PACKET::PACKET_STRUCT& sendPacketStructure);

					bool ReadFrom(char* const sReceiveBuffer, uint16_t& iRecvBytes);
					bool ReadFrom(NETWORK::USER_SESSION::USER_SERVER::OVERLAPPED_EX& receiveOverlapped);

					virtual bool SendCompletion(const uint16_t iSendBytes) override;

				};

				namespace RELIABLE {
					struct ReliableData : public FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::BaseData<ReliableData> {
					public:
						NETWORK::PACKET::PACKET_STRUCT m_packet;
						FUNCTIONS::SOCKETADDRESS::SocketAddress m_sendToAddress;

					public:
						ReliableData() {};
						ReliableData(const NETWORK::PACKET::PACKET_STRUCT& packet, const FUNCTIONS::SOCKETADDRESS::SocketAddress& sendToAddress) : m_packet(packet), m_sendToAddress(sendToAddress) {};

					};

					class ReliableUDP {
						const size_t REPEAT_COUNT_FOR_RELIABLE_SEND = 12;
					private:
						std::condition_variable m_sendCompleteNotify;
						std::condition_variable m_newReliableDataNotify;
						std::mutex m_reliableProcessorMutex;
						std::thread m_reliableThread;
						int16_t m_iThreadRunState;

						FUNCTIONS::CIRCULARQUEUE::CircularQueue<ReliableData*> m_reliableDataQueue;

						std::condition_variable m_newCachedDataNotify;
						std::mutex m_cacheReliableProcessorMutex;
						std::thread m_cacheReliableDataThread;
						int16_t m_iSendCompletedPacketNumber;

						std::unordered_map<int, ReliableData*> m_cacheReliableDataList;

					private:
						UDPIPSocket* const m_pUDPSocket;

					private:
						void ReliableProcessor();
						void CacheReliableDataProcessor();

					public:
						ReliableUDP(UDPIPSocket* const pUDPSocket);
						~ReliableUDP();

					public:
						ReliableData* AddReliableData(const NETWORK::PACKET::PACKET_STRUCT& packet, const FUNCTIONS::SOCKETADDRESS::SocketAddress& sendToAddress) {
							auto pResult = m_reliableDataQueue.Push(new ReliableData(packet, sendToAddress));
							m_newReliableDataNotify.notify_all();

							return pResult;
						}
					
						inline void Notify_SendComplete(const int16_t iSendCompletedPacketNumber) {
							m_sendCompleteNotify.notify_all();
							InterlockedExchange16(&m_iSendCompletedPacketNumber, iSendCompletedPacketNumber);
						}
						inline void Notify_NewReliableData() { m_newReliableDataNotify.notify_all(); }

					};
				}
			}
		}
	}
}