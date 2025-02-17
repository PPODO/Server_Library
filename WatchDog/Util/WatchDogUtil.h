#pragma once
#define NOMINMAX
#include <flatbuffers/flatbuffers.h>
#include "../../Network/Packet/Serialization/serialization.hpp"
#include "Packet/watchdog_data_define_generated.h"
#include "Packet/watchdog_client_end_request_generated.h"
#include "Packet/watchdog_client_end_result_generated.h"

using namespace SERVER::NETWORK::PACKET;
using namespace SERVER::NETWORK::PACKET::UTIL::SERIALIZATION;

namespace SERVER {
	namespace WATCHDOG {
		namespace UTIL {
			static PACKET_STRUCT CreateWatchDogClientInformationPacket(flatbuffers::FlatBufferBuilder& builder, const std::string& sProgramName = "", const std::string& sProgramPath = "", const std::string& sDumpFilePath = "", const std::string& sCommandLineArgv = "", const bool bEnableRestart = false) {
				return Serialize<WatchDogPacket::WatchDogClientInformation>(builder, WatchDogPacket::PacketType::PacketType_WatchDogStart, WatchDogPacket::CreateWatchDogClientInformation(builder, builder.CreateString(sProgramName), builder.CreateString(sProgramPath), builder.CreateString(sDumpFilePath), builder.CreateString(sCommandLineArgv), bEnableRestart));
			}

			static PACKET_STRUCT CreateWatchDogClientEndRequestPacket(flatbuffers::FlatBufferBuilder& builder, const bool bEnableRestart = false) {
				return Serialize<WatchDogPacket::WatchDogClientEndRequest>(builder, WatchDogPacket::PacketType::PacketType_WatchDogEndRequest, WatchDogPacket::CreateWatchDogClientEndRequest(builder, bEnableRestart));
			}

			static PACKET_STRUCT CreateWatchDogClientEndResultPacket(flatbuffers::FlatBufferBuilder& builder, const WatchDogPacket::RequestMessageType requestMessageType) {
				return Serialize<WatchDogPacket::WatchDogClientEndResult>(builder, WatchDogPacket::PacketType::PacketType_WatchDogEnd, WatchDogPacket::CreateWatchDogClientEndResult(builder, requestMessageType));
			}

		}
	}
}