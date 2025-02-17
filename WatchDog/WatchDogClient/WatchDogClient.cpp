#include "WatchDogClient.hpp"
#include "../Util/WatchDogUtil.h"

using namespace SERVER::NETWORKMODEL::EVENTSELECT;
using namespace SERVER::WATCHDOG::CLIENT;
using namespace SERVER::WATCHDOG;

#define MAX_FLATBUFFER_LENGTH 2048

CWatchDogClient::CWatchDogClient(const std::string& sCommandLineArgv, const bool bEnableRestart) : EventSelect(1, m_packetProcessor), m_flatbufferBuilder(MAX_FLATBUFFER_LENGTH) {
	std::string sModuleFileName(MAX_PATH, ' ');
	GetModuleFileNameA(NULL, &sModuleFileName.at(0), MAX_PATH);

	size_t iProgramNameOffset = sModuleFileName.find_last_of('\\');

	m_clientInformation.m_sProgramName = sModuleFileName.substr(iProgramNameOffset + 1);
	m_clientInformation.m_sProgramPath = sModuleFileName.substr(0, iProgramNameOffset);
	m_clientInformation.m_sDumpFilePath = SERVER::FUNCTIONS::MINIDUMP::Minidump::GetDumpFilePathA();
	m_clientInformation.m_sCommandLineArgv = sCommandLineArgv;
	m_clientInformation.m_bEnableRestart = bEnableRestart;

	m_packetProcessor.emplace(WatchDogPacket::PacketType_WatchDogEnd, std::bind(&CWatchDogClient::WatchDogEndResult, this, std::placeholders::_1));
}

bool SERVER::WATCHDOG::CLIENT::CWatchDogClient::Initialize(const EPROTOCOLTYPE protocolType, FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress) {
	if (EventSelect::Initialize(protocolType, serverAddress)) {
		const auto& packetStruct = UTIL::CreateWatchDogClientInformationPacket(m_flatbufferBuilder, m_clientInformation.m_sProgramName, m_clientInformation.m_sProgramPath, m_clientInformation.m_sDumpFilePath, m_clientInformation.m_sCommandLineArgv, m_clientInformation.m_bEnableRestart);
		
		return Send(packetStruct);
	}
	return false;
}

void SERVER::WATCHDOG::CLIENT::CWatchDogClient::Destroy() {
	EventSelect::Destroy();
}

void SERVER::WATCHDOG::CLIENT::CWatchDogClient::BeginDestroy(const bool bRestart) {
	Send(UTIL::CreateWatchDogClientEndRequestPacket(m_flatbufferBuilder, bRestart));
}

void SERVER::WATCHDOG::CLIENT::CWatchDogClient::WatchDogEndResult(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData) {
	if (pPacketData && m_OnDestroyCallback) {
		if (const auto pWatchDogEndResultData = WatchDogPacket::GetWatchDogClientEndResult(pPacketData->m_packetData->m_sPacketData)) {
			if (pWatchDogEndResultData->result_message() == WatchDogPacket::RequestMessageType::RequestMessageType_Succeeded)
				m_OnDestroyCallback();
		}
	}
}
