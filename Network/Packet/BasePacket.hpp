#pragma once
#include <Functions/Log/Log.hpp>
#include <Functions/CircularQueue/CircularQueue.hpp>
#include <Functions/MemoryPool/MemoryPool.h>

#include <Network/NetworkProtocol/Socket/Socket.hpp>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <string>
#include <climits>

namespace SERVER {
	namespace NETWORK {
		namespace PACKET {
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

			struct PACKET_STRUCT {
				static const size_t BUFFER_LENGTH = SERVER::NETWORK::PROTOCOL::BSD_SOCKET::MAX_BUFFER_SIZE * 2;
			public:
				PACKET_INFORMATION m_packetInfo;
				char m_sPacketData[BUFFER_LENGTH];

			public:
				PACKET_STRUCT() : m_packetInfo(), m_sPacketData("\0") {};
				PACKET_STRUCT(const PACKET_INFORMATION& packetInfo, const std::string& sPacketData) : m_packetInfo(packetInfo) {
					strcpy_s(m_sPacketData, BUFFER_LENGTH, sPacketData.c_str());
				};
			};

			class BasePacket {
				friend boost::serialization::access;
			public:
				const uint8_t m_iPacketType;
				uint8_t m_iMessageType;

			protected:
				template<typename Archive>
				void serialize(Archive& ar, unsigned int iVersion) {
					ar& m_iMessageType;
				}

			public:
				BasePacket(const uint8_t iPacketType, const uint8_t iMessageType) : m_iPacketType(iPacketType), m_iMessageType(iMessageType) {};
				virtual ~BasePacket() = 0;
			};

			template<typename T>
			class Packet : public BasePacket, public FUNCTIONS::MEMORYMANAGER::MemoryManager<T> {
			public:
				Packet(const uint8_t iPacketType, const uint8_t iMessageType) : BasePacket(iPacketType, iMessageType) {};
			};

			class PacketQueueData : public FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::BaseData<PacketQueueData, 500> {
			public:
				void* m_pOwner;
				PACKET_STRUCT m_packetData;

			public:
				PacketQueueData() : m_pOwner(nullptr) {};
				PacketQueueData(void* const pOwner, const PACKET_STRUCT& packetData) : m_pOwner(pOwner), m_packetData(packetData) {}

			};

			namespace UTIL {
				static PACKET_STRUCT Serialize(const BasePacket& packet) {

					std::string sTempBuffer;
					boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> outStream(sTempBuffer);
					boost::archive::binary_oarchive outArchive(outStream, boost::archive::no_header);

					outArchive << packet;

					return  PACKET_STRUCT(PACKET_INFORMATION(packet.m_iPacketType, sTempBuffer.length()), sTempBuffer);
				}

				static void Deserialize(const PACKET_STRUCT& inPacketData, BasePacket& outputPacketResult) {
					boost::iostreams::stream_buffer<boost::iostreams::basic_array_source<char>> inStream(inPacketData.m_sPacketData, inPacketData.m_packetInfo.m_iPacketDataSize);
					boost::archive::binary_iarchive inArchive(inStream, boost::archive::no_header);

					inArchive >> outputPacketResult;
				}
			}
		}
	}
}