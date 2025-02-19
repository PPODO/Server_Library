#pragma once
#define NOMINMAX
#include <flatbuffers/flatbuffers.h>
#include "../../Network/Packet/Serialization/serialization.hpp"
#include "Packet/watchdog_data_define_generated.h"
#include "Packet/watchdog_client_terminated_generated.h"
#include "Packet/watchdog_dump_transmit_generated.h"
#include "Packet/watchdog_ping_generated.h"

using namespace SERVER::NETWORK::PACKET;
using namespace SERVER::NETWORK::PACKET::UTIL::SERIALIZATION;

namespace SERVER {
	namespace WATCHDOG {
		namespace UTIL {
			static PACKET_STRUCT CreateWatchDogClientInformationPacketWithoutDiscordBot(flatbuffers::FlatBufferBuilder& builder, const std::string& sProgramName = "") {
				return Serialize<WatchDogPacket::WatchDogClientInformation>(
					builder, WatchDogPacket::PacketType::PacketType_NewProcessDetected, 
					WatchDogPacket::CreateWatchDogClientInformation(builder, builder.CreateString(sProgramName)));
			}

			static PACKET_STRUCT CreateWatchDogClientInformationPacketWithDiscordBot(flatbuffers::FlatBufferBuilder& builder, const std::string& sProgramName = "", const uint64_t iDiscordBotChannelID = 0) {
				return Serialize<WatchDogPacket::WatchDogClientInformation>(
					builder, WatchDogPacket::PacketType::PacketType_NewProcessDetected, 
					WatchDogPacket::CreateWatchDogClientInformation(builder, builder.CreateString(sProgramName), true, iDiscordBotChannelID));
			}

			static PACKET_STRUCT CreateWatchDogClientEndPacket(flatbuffers::FlatBufferBuilder& builder, const bool bHasDump) {
				return Serialize<WatchDogPacket::WatchDogClientTerminated>(builder, WatchDogPacket::PacketType::PacketType_ProcessTerminated, WatchDogPacket::CreateWatchDogClientTerminated(builder, bHasDump));
			}

			static PACKET_STRUCT CreateWatchDogDumpTransmitPacket(flatbuffers::FlatBufferBuilder& builder, const WatchDogPacket::PacketType packetType, const WatchDogPacket::DumpTransmitStatus transmitStatus, const std::string& sProgramName, const std::string& sDumpFileName, const std::vector<uint8_t>& sDumpFileData) {
				return Serialize<WatchDogPacket::DumpTransmitPacket>(builder, packetType, WatchDogPacket::CreateDumpTransmitPacket(builder, transmitStatus, builder.CreateString(sProgramName.c_str()), builder.CreateString(sDumpFileName.c_str()), builder.CreateVector(sDumpFileData)));
			}

			static PACKET_STRUCT CreatePingPacket(flatbuffers::FlatBufferBuilder& builder, const float fCPUUsage = 0.f, const double fMemoryUsage = 0.f, const uint64_t iTotalMemorySize = 0, const int32_t iConnectedUserCount = -1) {
				return Serialize<WatchDogPacket::PingPacket>(builder, WatchDogPacket::PacketType::PacketType_Ping, WatchDogPacket::CreatePingPacket(builder, fCPUUsage, fMemoryUsage, iTotalMemorySize, iConnectedUserCount));
			}

		}
	}
}