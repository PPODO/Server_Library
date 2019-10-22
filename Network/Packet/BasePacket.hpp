#pragma once
#include <Functions/Functions/Log/Log.h>
#include <Functions/Functions/CircularQueue/CircularQueue.hpp>
#include <Functions/Functions/MemoryPool/MemoryPool.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <string>
#include <climits>

namespace NETWORK {
	namespace PACKET {
		struct PACKET_STRUCTURE;
	}

	namespace UTIL {
		namespace PACKET {
			template<typename T>
			NETWORK::PACKET::PACKET_STRUCTURE Serialize(const T& Packet);
		}
	}

	namespace PACKET {
		namespace DETAIL {
			struct PACKET_INFORMATION {
			public:
				uint8_t m_PacketType;
				uint8_t m_MessageType;
				uint16_t m_PacketSize;
				uint16_t m_PacketNumber;

			public:
				PACKET_INFORMATION() noexcept : m_PacketType(0), m_MessageType(0), m_PacketSize(USHRT_MAX), m_PacketNumber(0) {};
				PACKET_INFORMATION(const uint8_t& PacketType, const uint8_t& MessageType, const uint16_t& PacketSize, const uint16_t& PacketNumber) noexcept : m_PacketType(PacketType), m_MessageType(MessageType), m_PacketSize(PacketSize), m_PacketNumber(PacketNumber) {};

			public:
				static size_t GetSize() { return sizeof(PACKET_INFORMATION); }

			};
		}

		struct PACKET_STRUCTURE {
		public:
			DETAIL::PACKET_INFORMATION m_PacketInformation;
			char m_PacketData[2048];

		public:
			PACKET_STRUCTURE() noexcept : m_PacketInformation(), m_PacketData() {};
			PACKET_STRUCTURE(const DETAIL::PACKET_INFORMATION& PacketInfo, const std::string& PacketData) noexcept : m_PacketInformation(PacketInfo) {
				CopyMemory(m_PacketData, PacketData.c_str(), PacketData.length());
			};
			
		};

		namespace BASEPACKET {
			struct CBasePacket {
				template<typename T>
				friend NETWORK::PACKET::PACKET_STRUCTURE UTIL::PACKET::Serialize(const T& Packet);
			public:
				const uint8_t m_PacketType;
				const uint8_t m_MessageType;

			public:
				CBasePacket(const uint8_t& PacketType, const uint8_t& MessageType) : m_PacketType(PacketType), m_MessageType(MessageType) {};
				virtual ~CBasePacket() = 0 {};

			};
		}

		template<typename T>
		struct CPacket : public BASEPACKET::CBasePacket, public FUNCTIONS::MEMORYMANAGER::CMemoryManager<T> {
		public:
			CPacket(const uint8_t& PacketType, const uint8_t& MessageType = 0) : CBasePacket(PacketType, MessageType) {};

		};
	}

	namespace UTIL {
		namespace PACKET {
			template<typename T>
			NETWORK::PACKET::PACKET_STRUCTURE Serialize(const T& Packet) {
				if (!std::is_base_of<NETWORK::PACKET::BASEPACKET::CBasePacket, T>()) {
					FUNCTIONS::LOG::CLog::WriteLog(L"");
					return NETWORK::PACKET::PACKET_STRUCTURE();
				}

				std::string TempBuffer;
				{
					boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> OutStream(TempBuffer);
					boost::archive::binary_oarchive oa(OutStream, boost::archive::no_header);
					oa << Packet;
				}

				const NETWORK::PACKET::BASEPACKET::CBasePacket* Temp = reinterpret_cast<const NETWORK::PACKET::BASEPACKET::CBasePacket*>(&Packet);
				return NETWORK::PACKET::PACKET_STRUCTURE(NETWORK::PACKET::DETAIL::PACKET_INFORMATION(Temp->m_PacketType, Temp->m_MessageType, TempBuffer.length(), 1), TempBuffer);
			}

			template<typename T>
			void DeSerialize(const NETWORK::PACKET::PACKET_STRUCTURE& PacketStructure, T& Packet) {
				if (!std::is_base_of<NETWORK::PACKET::BASEPACKET::CBasePacket, T>()) {
					FUNCTIONS::LOG::CLog::WriteLog(L"");
					return;
				}

				boost::iostreams::stream_buffer<boost::iostreams::basic_array_source<char>> InStream(PacketStructure.m_PacketData, PacketStructure.m_PacketInformation.m_PacketSize);
				boost::archive::binary_iarchive ia(InStream, boost::archive::no_header);
				ia >> Packet;
			}

		}
	}
}

namespace FUNCTIONS {
	namespace CIRCULARQUEUE {
		namespace QUEUEDATA {
			struct CPacketQueueData : public QUEUEDATA::DETAIL::BaseData<CPacketQueueData, 500> {
			public:
				NETWORK::PACKET::PACKET_STRUCTURE m_PacketStructure;
				void* m_Owner;

			public:
				CPacketQueueData() : m_Owner(nullptr) { ZeroMemory(&m_PacketStructure, sizeof(NETWORK::PACKET::PACKET_STRUCTURE)); };
				CPacketQueueData(void* const Owner, const NETWORK::PACKET::PACKET_STRUCTURE& PacketStructure) : m_Owner(Owner), m_PacketStructure(PacketStructure) {};

			};
		}
	}
}