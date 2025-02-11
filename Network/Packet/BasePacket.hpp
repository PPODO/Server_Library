#pragma once
#include "../../Functions/Log/Log.hpp"
#include "../../Functions/CircularQueue/CircularQueue.hpp"
#include "../../Functions/MemoryPool/MemoryPool.h"

#include "../NetworkProtocol/Socket/Socket.hpp"
#include <boost/serialization/serialization.hpp>

#include <string>
#include <climits>

namespace SERVER {
	namespace NETWORK {
		namespace PACKET {
#pragma pack(push, 1)
			struct PACKET_INFORMATION {
			public:
				uint8_t m_iPacketType;
				uint16_t m_iPacketDataSize;
				uint16_t m_iPacketNumber;

			public:
 				PACKET_INFORMATION() : m_iPacketType(0), m_iPacketDataSize(0), m_iPacketNumber(0) {};
				PACKET_INFORMATION(const uint8_t iPacketType, const uint16_t iPacketDataSize) : m_iPacketType(iPacketType), m_iPacketDataSize(iPacketDataSize), m_iPacketNumber(0) {};

			public:
				static size_t GetStructSize() { return sizeof(PACKET_INFORMATION); }
			};
#pragma pack(pop)

#pragma pack(push, 1)
			struct PACKET_STRUCT : public SERVER::FUNCTIONS::MEMORYMANAGER::CMemoryManager<PACKET_STRUCT, 500> {
				static const unsigned long BUFFER_LENGTH = SERVER::NETWORK::PROTOCOL::BSD_SOCKET::MAX_BUFFER_SIZE * 2;
			public:
				PACKET_INFORMATION m_packetInfo;
				char m_sPacketData[BUFFER_LENGTH];

			public:
				PACKET_STRUCT() : m_packetInfo(), m_sPacketData() { ZeroMemory(m_sPacketData, BUFFER_LENGTH); };

				PACKET_STRUCT(const PACKET_INFORMATION& packetInfo) : m_packetInfo(packetInfo) {
					ZeroMemory(m_sPacketData, BUFFER_LENGTH);
				}

				PACKET_STRUCT(const PACKET_INFORMATION& packetInfo, const std::string& sPacketData) : m_packetInfo(packetInfo) {
					ZeroMemory(m_sPacketData, BUFFER_LENGTH);
					CopyMemory(m_sPacketData, sPacketData.c_str(), sPacketData.length());
				};
				
				PACKET_STRUCT(const PACKET_INFORMATION& packetInfo, const uint8_t* sPacketData) : m_packetInfo(packetInfo) {
					ZeroMemory(m_sPacketData, BUFFER_LENGTH);
					CopyMemory(m_sPacketData, sPacketData, packetInfo.m_iPacketDataSize);
				}

				PACKET_STRUCT(const PACKET_STRUCT& rhs) : m_packetInfo(rhs.m_packetInfo) {
					ZeroMemory(m_sPacketData, BUFFER_LENGTH);
					CopyMemory(m_sPacketData, rhs.m_sPacketData, rhs.m_packetInfo.m_iPacketDataSize);
				}

			};
#pragma pack(pop)

			struct BasePacket {
				friend boost::serialization::access;
			public:
				const uint8_t m_iPacketType;
				uint32_t m_iMessageType;

			protected:
				template<typename Archive>
				void serialize(Archive& ar, unsigned int iVersion) {
					ar& m_iMessageType;
				}

			public:
				BasePacket(const uint8_t iPacketType, const uint32_t iMessageType) : m_iPacketType(iPacketType), m_iMessageType(iMessageType) {};
				virtual ~BasePacket() = 0 {}
			};

			template<typename T>
			struct Packet : public BasePacket, public FUNCTIONS::MEMORYMANAGER::CMemoryManager<T> {
			public:
				Packet(const uint8_t iPacketType, const uint32_t iMessageType) : BasePacket(iPacketType, iMessageType) {};
			};

			struct PacketQueueData : public FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::BaseData<PacketQueueData, 500> {
			public:
				void* m_pOwner;
				std::shared_ptr<PACKET_STRUCT> m_packetData;

			public:
				PacketQueueData() : m_pOwner(nullptr) {};
				PacketQueueData(void* const pOwner, const PACKET_STRUCT& packetData) : m_pOwner(pOwner), m_packetData(std::shared_ptr<PACKET_STRUCT>(new PACKET_STRUCT(packetData))) {}
				PacketQueueData(void* const pOwner, PACKET_STRUCT* packetData) : m_pOwner(pOwner), m_packetData(packetData) {}

			};

			
		}
	}
}