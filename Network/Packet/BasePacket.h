#pragma once
#include <Functions/Functions/MemoryPool/MemoryPool.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>

namespace NETWORK {
	namespace PACKET {
		namespace BASEPACKET {
			class CBasePacket {

			};
		}
	}

	namespace UTIL {
		namespace BASEPACKET {
			template<typename T>
			void Serialize(const T& Packet, std::string& Buffer) {
				if (!std::is_base_of<DETAIL::CBasePacket, T>()) {

				}

				{
					boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> OutStream(Buffer);
					boost::archive::binary_oarchive oa(OutStream, boost::archive::no_header);
					oa << Packet;
				}
			}

			template<typename T>
			void DeSerialize(const char* Buffer, const uint16_t& BufferLength, T& Packet) {
				if (!std::is_base_of<NETWORK::BASEPACKET::CBasePacket, T>()) {
					FUNCTIONS::LOG::CLog::WriteLog(L"");
					return;
				}

				boost::iostreams::stream_buffer<boost::iostreams::basic_array_source<char>> InStream(Buffer, BufferLength);
				boost::archive::binary_iarchive ia(InStream, boost::archive::no_header);
				ia >> Packet;
			}

		}
	}
}