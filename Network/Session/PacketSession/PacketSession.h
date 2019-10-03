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
						// ���� �׷� ��� ���� ��Ŷ ���ۿ��� ��Ŷ �����ŭ�� �����͸� �о��ٴ� �������� ĳ��Ʈ�� ��.
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