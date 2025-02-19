#pragma once
#define NOMINMAX
#define _WINSOCKAPI_
#include <flatbuffers/flatbuffers.h>
#include <Network/NetworkProtocol/TCPSocket.hpp>
#include <Functions/CircularQueue/CircularQueue.hpp>
#include <atomic>
#include <mutex>

#define MAX_FILE_NAME 50

namespace SERVER {
	namespace WATCHDOG {
		namespace DUMPTRANSMIT {
			struct FDumpTransmitRequestQueueData : public SERVER::FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::BaseData<FDumpTransmitRequestQueueData> {
			public:
				CHAR m_sProgramName[MAX_FILE_NAME];
				CHAR m_sDumpFilePath[MAX_PATH];
				CHAR m_sDumpFileName[MAX_FILE_NAME];

			public:
				FDumpTransmitRequestQueueData(const char* const sProgramName, const char* const sDumpFilePath, const char* const sDumpFileName) {
					ZeroMemory(m_sProgramName, MAX_FILE_NAME);
					CopyMemory(m_sProgramName, sProgramName, MAX_FILE_NAME);

					ZeroMemory(m_sDumpFilePath, MAX_PATH);
					CopyMemory(m_sDumpFilePath, sDumpFilePath, MAX_PATH);

					ZeroMemory(m_sDumpFileName, MAX_FILE_NAME);
					CopyMemory(m_sDumpFileName, sDumpFileName, MAX_FILE_NAME);
				}

			};

			class CDumpTransmit {
			private:
				flatbuffers::FlatBufferBuilder m_flatbuffer;

				SERVER::NETWORK::PROTOCOL::TCP::TCPIPSocket m_dumpTransmitSocket;

				std::atomic_bool m_bDumpFileTransmitThreadState;
				std::thread m_dumpFileTransmitThread;

				std::mutex m_csForTransmitRequestQueue;
				std::condition_variable m_cvForTransmitRequestQueue;
				SERVER::FUNCTIONS::CIRCULARQUEUE::CircularQueue<FDumpTransmitRequestQueueData*> m_dumpTransmitRequestQueue;

			private:
				void DumpFileTransmitThread();

			public:
				CDumpTransmit();

				bool Initialize(SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress);
				void Destroy();

			public:
				void AddNewDumpTransmitQueueData(const std::string& sProgramName, const std::string& sDumpFilePath, const std::string& sDumpFileName);

				bool HasDumpFile(const std::string& sDumpFilePath, const DWORD iPID, std::string& sOutDumpFileName);

			};
		}
	}
}