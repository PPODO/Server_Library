#pragma once
#include <Functions/Functions/CircularQueue/CircularQueue.hpp>
#include <Network/Socket/Socket.h>
#include <Network/Packet/BasePacket.hpp>

namespace NETWORK {
	namespace SESSION {
		namespace NETWORKSESSION {
			class CNetworkSession;
		}

		namespace PACKETSESSION {
			class CPacketSession {
			private:
				PACKET::PACKET_STRUCTURE m_LastPacketReceived;

			public:
				explicit CPacketSession();
				virtual ~CPacketSession();

			private:
				inline bool UpdateLastReceivedPacketInformation(char* const DataBuffer, uint16_t& CurrentReceivedBytes) {
					using namespace NETWORK::PACKET;

					if (CurrentReceivedBytes >= DETAIL::PACKET_INFORMATION::GetSize()) {
						// 만약 그럴 경우 현재 패킷 버퍼에는 패킷 사이즈만큼의 데이터를 읽었다는 뜻임으로 캐스트를 함.
						if (DETAIL::PACKET_INFORMATION * PacketInfo = reinterpret_cast<DETAIL::PACKET_INFORMATION*>(DataBuffer)) {
							m_LastPacketReceived.m_PacketInformation = *PacketInfo;
							return true;
						}
						throw std::bad_cast();
					}
				}
				inline void ClearProcessedData(char* const DataBuffer, const uint16_t& BufferStartPosition, const uint16_t& ClearLength, uint16_t& CurrentReceivedBytes, const uint16_t& MinusValue) {
					using namespace NETWORK::PACKET;

					if (DataBuffer) {
						MoveMemory(DataBuffer, DataBuffer + BufferStartPosition, ClearLength);
						CurrentReceivedBytes -= MinusValue;
					}
				}

			protected:
				const FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const ReceivedDataDeSerialization(char* const DataBuffer, uint16_t& CurrentReceivedBytes);

			};
		}
	}
}