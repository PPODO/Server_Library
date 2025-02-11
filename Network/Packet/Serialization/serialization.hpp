#pragma once
#include "../BasePacket.hpp"
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/unordered_map.hpp>

#define NOMINMAX
#include <flatbuffers/flatbuffers.h>
#undef NOMINMAX

namespace SERVER {
	namespace NETWORK {
		namespace PACKET{
			namespace UTIL {
				namespace SERIALIZATION {
					template<typename T>
					SERVER::NETWORK::PACKET::PACKET_STRUCT Serialize(const SERVER::NETWORK::PACKET::BasePacket& packet) {
						std::string sTempBuffer;
						{
							boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> outStream(sTempBuffer);
							boost::archive::binary_oarchive outArchive(outStream, boost::archive::no_header);

							outArchive << *reinterpret_cast<const T*>(&packet);
						}

						return PACKET_STRUCT(PACKET_INFORMATION(packet.m_iPacketType, sTempBuffer.length()), sTempBuffer);
					}

					template<typename T>
					PACKET::PACKET_STRUCT Serialize(flatbuffers::FlatBufferBuilder& builder, const uint8_t iPacketType, flatbuffers::Offset<T>&& data) {
						builder.Finish(data);

						PACKET::PACKET_STRUCT ret(PACKET_INFORMATION(iPacketType, builder.GetBufferSpan().size()), builder.GetBufferPointer());

						builder.Clear();

						return ret;
					}

					template<typename T>
					void Deserialize(const SERVER::NETWORK::PACKET::PACKET_STRUCT& inPacketData, T& outputPacketResult) {
						boost::iostreams::stream_buffer<boost::iostreams::basic_array_source<char>> inStream
						(inPacketData.m_sPacketData, inPacketData.m_packetInfo.m_iPacketDataSize);
						boost::archive::binary_iarchive inArchive(inStream, boost::archive::no_header);

						inArchive >> outputPacketResult;
					}
				}
			}
		}
	}
}