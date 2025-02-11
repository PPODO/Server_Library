#include "WatchDogClient.hpp"
#include "../Util/WatchDogUtil.h"

using namespace SERVER::NETWORKMODEL::EVENTSELECT;
using namespace SERVER::WATCHDOG::CLIENT;
using namespace SERVER::WATCHDOG;

#define MAX_FLATBUFFER_LENGTH 2048

CWatchDogClient::CWatchDogClient(const std::string& sCommandLineArgv, const bool bEnableRestart) : EventSelect(0, PACKETPROCESSOR()), m_flatbufferBuilder(MAX_FLATBUFFER_LENGTH) {
	std::string sModuleFileName(MAX_PATH, ' ');
	GetModuleFileNameA(NULL, &sModuleFileName.at(0), MAX_PATH);

	size_t iProgramNameOffset = sModuleFileName.find_last_of('\\');

	m_clientInformation.m_sProgramName = sModuleFileName.substr(iProgramNameOffset + 1);
	m_clientInformation.m_sProgramPath = sModuleFileName.substr(0, iProgramNameOffset);
	m_clientInformation.m_sDumpFilePath = SERVER::FUNCTIONS::MINIDUMP::Minidump::GetDumpFilePathA();
	m_clientInformation.m_sCommandLineArgv = sCommandLineArgv;
	m_clientInformation.m_bEnableRestart = bEnableRestart;
}

bool SERVER::WATCHDOG::CLIENT::CWatchDogClient::Initialize(const EPROTOCOLTYPE protocolType, FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress) {
	if (EventSelect::Initialize(protocolType, serverAddress)) {
		const auto& packetStruct = UTIL::CreateWatchDogClientInformationPacket(m_flatbufferBuilder, m_clientInformation.m_sProgramName, m_clientInformation.m_sProgramPath, m_clientInformation.m_sDumpFilePath, m_clientInformation.m_sCommandLineArgv, m_clientInformation.m_bEnableRestart);
		
		return Send(packetStruct);
	}
	return false;
}

void SERVER::WATCHDOG::CLIENT::CWatchDogClient::Destroy() {
	Send(UTIL::CreateWatchDogClientEndPacket(m_flatbufferBuilder, true));

	EventSelect::Destroy();
}