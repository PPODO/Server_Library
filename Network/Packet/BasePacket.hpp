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
				uint16_t m_PacketSize;
				uint16_t m_PacketNumber;

			public:
				PACKET_INFORMATION() noexcept : m_PacketType(0), m_PacketSize(USHRT_MAX), m_PacketNumber(0) {};
				PACKET_INFORMATION(const uint8_t& PacketType, const uint16_t& PacketSize) noexcept : m_PacketType(PacketType), m_PacketSize(PacketSize), m_PacketNumber(0) {};

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
				friend boost::serialization::access;
			public:
				const uint8_t m_PacketType;
				uint8_t m_MessageType;

			protected:
				template<typename Archive>
				void serialize(Archive& ar, unsigned int Version) {
					ar& m_MessageType;
				}

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
				return NETWORK::PACKET::PACKET_STRUCTURE(NETWORK::PACKET::DETAIL::PACKET_INFORMATION(Temp->m_PacketType, TempBuffer.length()), TempBuffer);
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