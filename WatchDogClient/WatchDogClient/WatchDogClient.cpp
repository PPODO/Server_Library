#include "WatchDogClient.hpp"
#include "../Util/WatchDogUtil.h"
#include <Functions/FPSManagement/FPSManager.hpp>
#include <conio.h>
#include "psapi.h"

using namespace SERVER::NETWORKMODEL::EVENTSELECT;
using namespace SERVER::WATCHDOG::CLIENT;
using namespace SERVER::WATCHDOG;

#define MAX_FLATBUFFER_LENGTH 2048
#define BYTE_TO_MB(x) (((x / 1024.f) / 1024.f))

CWatchDogClient::CWatchDogClient(const std::string& sJsonFilePath) : EventSelect(1, m_packetProcessor), m_bWatchDogClientRunState(true), m_flatbufferBuilder(MAX_FLATBUFFER_LENGTH) {
	m_packetProcessor.emplace(WatchDogPacket::PacketType::PacketType_Ping, std::bind(&CWatchDogClient::Ping, this, std::placeholders::_1));

	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	if (GlobalMemoryStatusEx(&memInfo))
		m_iTotalPhysicalMemorySize = BYTE_TO_MB(memInfo.ullTotalPhys);
	else {
		SERVER::FUNCTIONS::LOG::Log::WriteLog(L"Config File Path Is Incorrect.");
		exit(-2);
	}

	if (!SetupConfig(sJsonFilePath))
		exit(-1);
}

bool CWatchDogClient::Initialize(const EPROTOCOLTYPE protocolType, FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress) {
	if (EventSelect::Initialize(protocolType, serverAddress) &&
		m_dumpTransmitInstance.Initialize(serverAddress)) {

		for (const auto& iterator : m_clientList) {
			if (iterator.m_iDiscordBotChannelID != 0)
				Send(UTIL::CreateWatchDogClientInformationPacketWithDiscordBot(m_flatbufferBuilder, iterator.m_sProgramName, iterator.m_iDiscordBotChannelID));
			else
				Send(UTIL::CreateWatchDogClientInformationPacketWithoutDiscordBot(m_flatbufferBuilder, iterator.m_sProgramName));
		}
		m_childProcessEventSelectWorkerThread = std::thread(std::bind(&CWatchDogClient::ChildProcessEventSelectWorkerThread, this));
	}
	return true;
}

void CWatchDogClient::Run() {
	EventSelect::Run();

	m_csForClientList.Lock();
	for (auto& iterator : m_clientList) {
		if (iterator.m_cpuUsageInformation.CanCalculate())
			iterator.m_cpuUsageInformation.GetCurrentCPUUsage(iterator.m_processInformation.hProcess);


	}
	m_csForClientList.UnLock();

	if (_kbhit()) {
		const int iKey = _getch();
		if (iKey == 'q' || iKey == 'Q')
			m_bWatchDogClientRunState = false;
	}
}

void CWatchDogClient::Destroy() {
	if (m_childProcessEventSelectWorkerThread.joinable())
		m_childProcessEventSelectWorkerThread.join();
	
	m_dumpTransmitInstance.Destroy();
	EventSelect::Destroy();
}

bool CWatchDogClient::SetupConfig(const std::string& sConfigJsonFilePath) {
	std::ifstream configJsonFileStream(sConfigJsonFilePath, std::ios::in);
	Json::Value jsonConfig;
	Json::Reader jsonReader;

	if (configJsonFileStream.is_open() && jsonReader.parse(configJsonFileStream, jsonConfig)) {
		for (auto& programInformation : jsonConfig["ProgramInformation"]) {
			auto&& clientInfo = std::move(FWatchDogClientInformation::ReadFromJson(programInformation));

			if (!CreateNewChildProcess(clientInfo))
				return false;
		}
	}
	else {
		SERVER::FUNCTIONS::LOG::Log::WriteLog(L"Config File Path Is Incorrect.");
		return false;
	}
}

bool CWatchDogClient::CreateNewChildProcess(FWatchDogClientInformation& clientInfo) {
	const std::string sCombine = clientInfo.m_sProgramPath + "\\" + clientInfo.m_sProgramName;

	STARTUPINFOA startInfo = { sizeof(STARTUPINFOA) };
	PROCESS_INFORMATION processInformation;

	if (clientInfo.m_hSharedMemory != NULL)
		CloseHandle(clientInfo.m_hSharedMemory);

	clientInfo.m_hSharedMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int32_t), SERVER::FUNCTIONS::UTIL::MBToUni(clientInfo.m_sProgramName + " IPC").c_str());
	if (CreateProcessA(sCombine.c_str(), const_cast<char*>(clientInfo.m_sCommandLineArgv.c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &startInfo, &processInformation) &&
		clientInfo.m_cpuUsageInformation.Init(processInformation.hProcess) && clientInfo.m_hSharedMemory != INVALID_HANDLE_VALUE) {
		
		clientInfo.m_processInformation = processInformation;

		SERVER::FUNCTIONS::LOG::Log::WriteLog(L"Client Process Started Successful : %ls", SERVER::FUNCTIONS::UTIL::MBToUni(clientInfo.m_sProgramName).c_str());

		m_csForClientList.Lock();
		m_clientList.push_back(std::move(clientInfo));
		m_csForClientList.UnLock();
		return true;
	}
	SERVER::FUNCTIONS::LOG::Log::WriteLog(L"Client Process Failed To Start : %ls", SERVER::FUNCTIONS::UTIL::MBToUni(clientInfo.m_sProgramName).c_str());
	return false;
}

void CWatchDogClient::ChildProcessEventSelectWorkerThread() {
	const size_t iChildProcessCount = m_clientList.size();

	std::vector<HANDLE> processHandles;
	for (const auto& iterator : m_clientList)
		processHandles.emplace_back(iterator.m_processInformation.hProcess);

	while (m_bWatchDogClientRunState) {
		DWORD iWaitResult = WaitForMultipleObjects(processHandles.size(), processHandles.data(), FALSE, INFINITE);

		if (iWaitResult >= WAIT_OBJECT_0 && iWaitResult < WAIT_OBJECT_0 + processHandles.size()) {
			const size_t iIndex = iWaitResult - WAIT_OBJECT_0;

			m_csForClientList.Lock();

			std::string sDumpFileName;
			bool bHasDump = m_dumpTransmitInstance.HasDumpFile(m_clientList[iIndex].m_sDumpFilePath, m_clientList[iIndex].m_processInformation.dwProcessId, sDumpFileName);
			if (bHasDump)
				m_dumpTransmitInstance.AddNewDumpTransmitQueueData(m_clientList[iIndex].m_sProgramName, m_clientList[iIndex].m_sDumpFilePath, sDumpFileName);
			
			Send(UTIL::CreateWatchDogClientEndPacket(m_flatbufferBuilder, bHasDump));
			
			if (m_clientList[iIndex].m_bEnableRestart) {
				if (CreateNewChildProcess(m_clientList[iIndex]))
					processHandles.emplace_back(m_clientList[iIndex].m_processInformation.hProcess);
			}

			CloseHandle(processHandles[iIndex]);

			m_clientList.erase(m_clientList.begin() + iIndex);

			m_csForClientList.UnLock();

			processHandles.erase(processHandles.begin() + iIndex);
		}
		else {
			SERVER::FUNCTIONS::LOG::Log::WriteLog(L"No Processes For Monitoring Have Been Detected. Exit The WatchDog.");
			m_bWatchDogClientRunState = false;
		}
	}
}

void CWatchDogClient::Ping(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData) {
	if (auto pPingPacket = WatchDogPacket::GetPingPacket(pPacketData->m_packetData->m_sPacketData)) {
		
		m_csForClientList.Lock();

		for (const auto& iterator : m_clientList) {
			int32_t iConnectedUserCount = -1;
			if (iterator.m_hSharedMemory != NULL) {
				if (auto pConnectedUserCountRef = (int32_t*)MapViewOfFile(iterator.m_hSharedMemory, FILE_MAP_READ, 0, 0, 0))
					iConnectedUserCount = *pConnectedUserCountRef;
			}

			float fMemoryUsage = -1.f;
			PROCESS_MEMORY_COUNTERS_EX processMemoryCounter;
			if (GetProcessMemoryInfo(iterator.m_processInformation.hProcess, (PROCESS_MEMORY_COUNTERS*)&processMemoryCounter, sizeof(processMemoryCounter)))
				fMemoryUsage = BYTE_TO_MB(processMemoryCounter.PrivateUsage);
			
			Send(UTIL::CreatePingPacket(m_flatbufferBuilder, iterator.m_cpuUsageInformation.m_fLastCPUUsage, fMemoryUsage, m_iTotalPhysicalMemorySize, iConnectedUserCount));
		}

		m_csForClientList.UnLock();
	}
}

