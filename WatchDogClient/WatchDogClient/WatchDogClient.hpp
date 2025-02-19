#pragma once
#define NOMINMAX
#define _WINSOCKAPI_
#include "../DumpTransmit/DumpTransmit.h"
#include "../../NetworkModel/EventSelect/EventSelect.hpp"
#include "../../Functions/Minidump/Minidump.hpp"
#include <flatbuffers/flatbuffers.h>
#include <json/json.h>
#include <vector>
#include <chrono>

namespace SERVER {
	namespace WATCHDOG {
		namespace CLIENT {
			struct FCPUUsageInformation {
			public:
				ULARGE_INTEGER m_iLastCPUTime, m_iLastSystemCPUTime, m_iLastUserCPUTime;
				int32_t m_iNumOfProcessors;

				double m_fLastCPUUsage;

				std::chrono::system_clock::time_point m_lastCalcuatedCPUUsageTime;

			public:
				FCPUUsageInformation() : m_iLastCPUTime(), m_fLastCPUUsage(), m_iLastSystemCPUTime(), m_iLastUserCPUTime(), m_iNumOfProcessors() {}

				bool Init(const HANDLE hProcessHandle) {
					m_lastCalcuatedCPUUsageTime = std::chrono::system_clock::now();

					SYSTEM_INFO systemInfo;
					GetSystemInfo(&systemInfo);
					m_iNumOfProcessors = systemInfo.dwNumberOfProcessors;

					FILETIME fTime, fSystemTime, fUserTime;

					if (!GetProcessTimes(hProcessHandle, &fTime, &fTime, &fSystemTime, &fUserTime)) 
						return false;
					
					memcpy(&m_iLastSystemCPUTime, &fSystemTime, sizeof(FILETIME));
					memcpy(&m_iLastUserCPUTime, &fUserTime, sizeof(FILETIME));

					return true;
				}

				double GetCurrentCPUUsage(const HANDLE hProcessHandle) {
					FILETIME fTime, fSystemTime, fUserTime;
					ULARGE_INTEGER iCurrentTime, iCurrentSystemTime, iCurrentUserTime;

					GetSystemTimeAsFileTime(&fTime);
					memcpy(&iCurrentTime, &fTime, sizeof(FILETIME));

					if (!GetProcessTimes(hProcessHandle, &fTime, &fTime, &fSystemTime, &fUserTime))
						return -1.0;

					memcpy(&iCurrentSystemTime, &fSystemTime, sizeof(FILETIME));
					memcpy(&iCurrentUserTime, &fUserTime, sizeof(FILETIME));


					m_fLastCPUUsage = (iCurrentSystemTime.QuadPart - m_iLastSystemCPUTime.QuadPart) + (iCurrentUserTime.QuadPart - m_iLastUserCPUTime.QuadPart);
					m_fLastCPUUsage /= (iCurrentTime.QuadPart - m_iLastCPUTime.QuadPart);
					m_fLastCPUUsage /= m_iNumOfProcessors;
					m_fLastCPUUsage *= 100.f;

					m_iLastCPUTime = iCurrentTime;
					m_iLastSystemCPUTime = iCurrentSystemTime;
					m_iLastUserCPUTime = iCurrentUserTime;

					return m_fLastCPUUsage;
				}

				bool CanCalculate() {
					if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_lastCalcuatedCPUUsageTime).count() >= 1000) {
						m_lastCalcuatedCPUUsageTime = std::chrono::system_clock::now();
						return true;
					}
					return false;

				}

			};

			struct FWatchDogClientInformation {
			public:
				PROCESS_INFORMATION m_processInformation;

				std::string m_sProgramName;
				std::string m_sProgramPath;
				std::string m_sDumpFilePath;
				std::string m_sCommandLineArgv;
				bool m_bEnableRestart;
				bool m_bProgramRunningState;

				uint64_t m_iDiscordBotChannelID;

				FCPUUsageInformation m_cpuUsageInformation;

				SIZE_T m_iVirtualMemoryUsage;

				HANDLE m_hSharedMemory;

			public:
				FWatchDogClientInformation() : m_iVirtualMemoryUsage(), m_hSharedMemory(NULL), m_processInformation(), m_sProgramName(), m_sProgramPath(), m_sDumpFilePath(), m_sCommandLineArgv(), m_bEnableRestart(false), m_bProgramRunningState(false), m_iDiscordBotChannelID(), m_cpuUsageInformation() {};
				
				static FWatchDogClientInformation ReadFromJson(Json::Value& root) {
					FWatchDogClientInformation programInfo;

					programInfo.m_sProgramName = root["ProgramName"].asCString();
					programInfo.m_sProgramPath = root["ProgramPath"].asCString();
					programInfo.m_sDumpFilePath = programInfo.m_sProgramPath;
					programInfo.m_sCommandLineArgv = root["CommandLineArgv"].asCString();

					programInfo.m_bEnableRestart = root["EnableRestart"].asBool();
					programInfo.m_bProgramRunningState = false;

					programInfo.m_iDiscordBotChannelID = root["DiscordBotChannelID"].asUInt64();

					return programInfo;
				}

			};
			
			class CWatchDogClient : public SERVER::NETWORKMODEL::EVENTSELECT::EventSelect {
			public:
				CWatchDogClient(const std::string& sJsonFilePath);

			public:
				virtual bool Initialize(const EPROTOCOLTYPE protocolType, FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress) override final;
				virtual void Run() override final;
				virtual void Destroy() override final;

			public:
				bool GetClientRunState() const { return m_bWatchDogClientRunState; }

			public:
				std::function<void()> m_OnDestroyCallback;

			private:
				bool SetupConfig(const std::string& sConfigJsonFilePath);
				bool CreateNewChildProcess(FWatchDogClientInformation& clientInfo);

				void ChildProcessEventSelectWorkerThread();

				void Ping(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData);

			private:
				DWORDLONG m_iTotalPhysicalMemorySize;

				std::atomic_bool m_bWatchDogClientRunState;
				PACKETPROCESSOR m_packetProcessor;
				
				SERVER::FUNCTIONS::CRITICALSECTION::CriticalSection m_csForClientList;
				std::vector<FWatchDogClientInformation> m_clientList;

				std::thread m_childProcessEventSelectWorkerThread;

				DUMPTRANSMIT::CDumpTransmit m_dumpTransmitInstance;

				flatbuffers::FlatBufferBuilder m_flatbufferBuilder;

			};

		}
	}
}