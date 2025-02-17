#pragma once
#define NOMINMAX
#define _WINSOCKAPI_
#include "../../NetworkModel/EventSelect/EventSelect.hpp"
#include "../../Functions/Minidump/Minidump.hpp"
#include <flatbuffers/flatbuffers.h>

namespace SERVER {
	namespace WATCHDOG {
		namespace CLIENT {
			struct FWatchDogClientInformation {
			public:
				std::string m_sProgramName;
				std::string m_sProgramPath;
				std::string m_sDumpFilePath;
				std::string m_sCommandLineArgv;
				bool m_bEnableRestart;
				bool m_bProgramRunningState;

			public:
				FWatchDogClientInformation() : m_sProgramName(), m_sProgramPath(), m_sDumpFilePath(), m_sCommandLineArgv(), m_bEnableRestart(false), m_bProgramRunningState(false) {};
				FWatchDogClientInformation(const std::string& sProgramName, const std::string& sProgramPath, const std::string& sDumpFilePath, const std::string& sCommandLineArgv, const bool bEnableResatrt)
					:m_sProgramName(sProgramName), m_sProgramPath(sProgramPath), m_sDumpFilePath(sDumpFilePath), m_sCommandLineArgv(sCommandLineArgv), m_bEnableRestart(bEnableResatrt), m_bProgramRunningState(true) {
				}

			};
			
			class CWatchDogClient : public SERVER::NETWORKMODEL::EVENTSELECT::EventSelect {
			public:
				CWatchDogClient(const std::string& sCommandLineArgv, const bool bEnableRestart);

			public:
				virtual bool Initialize(const EPROTOCOLTYPE protocolType, FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress) override final;
				virtual void Destroy() override final;

			public:
				void BeginDestroy(const bool bRestart);

			public:
				std::function<void()> m_OnDestroyCallback;

			private:
				void WatchDogEndResult(SERVER::NETWORK::PACKET::PacketQueueData* const pPacketData);

			private:
				PACKETPROCESSOR m_packetProcessor;

				FWatchDogClientInformation m_clientInformation;

				flatbuffers::FlatBufferBuilder m_flatbufferBuilder;

			};

		}
	}
}