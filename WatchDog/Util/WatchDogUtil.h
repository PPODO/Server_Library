#pragma once
#define NOMINMAX
#include <flatbuffers/flatbuffers.h>
#include "../../Network/Packet/Serialization/serialization.hpp"
#include "Packet/watchdog_data_define_generated.h"
#include "Packet/watchdog_client_end_reqeust_generated.h"

using namespace SERVER::NETWORK::PACKET;
using namespace SERVER::NETWORK::PACKET::UTIL::SERIALIZATION;

namespace SERVER {
	namespace WATCHDOG {
		namespace UTIL {
			static PACKET_STRUCT CreateWatchDogClientInformationPacket(flatbuffers::FlatBufferBuilder& builder, const std::string& sProgramName = "", const std::string& sProgramPath = "", const std::string& sDumpFilePath = "", const std::string& sCommandLineArgv = "", const bool bEnableRestart = false) {
				return Serialize<WatchDogPacket::WatchDogClientInformation>(builder, WatchDogPacket::PacketType::PacketType_WatchDogStart, WatchDogPacket::CreateWatchDogClientInformation(builder, builder.CreateString(sProgramName), builder.CreateString(sProgramPath), builder.CreateString(sDumpFilePath), builder.CreateString(sCommandLineArgv), bEnableRestart));
			}

			static PACKET_STRUCT CreateWatchDogClientEndPacket(flatbuffers::FlatBufferBuilder& builder, const bool bEnableRestart = false) {
				return Serialize<WatchDogPacket::WatchDogClientEndRequest>(builder, WatchDogPacket::PacketType::PacketType_WatchDogEnd, WatchDogPacket::CreateWatchDogClientEndRequest(builder, bEnableRestart));
			}

		}
	}
}