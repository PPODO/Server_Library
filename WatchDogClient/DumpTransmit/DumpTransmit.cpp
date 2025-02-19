#include "DumpTransmit.h"
#include "../Util/WatchDogUtil.h"
#include <functional>
#include <chrono>

#define MAX_MESSAGE_BUFFER_LENGTH 1024

using namespace SERVER::WATCHDOG::DUMPTRANSMIT;

CDumpTransmit::CDumpTransmit() : m_bDumpFileTransmitThreadState(true), m_flatbuffer(MAX_MESSAGE_BUFFER_LENGTH * 2) {
}

bool CDumpTransmit::Initialize(SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress) {
	if (m_dumpTransmitSocket.Connect(serverAddress)) {
		m_dumpFileTransmitThread = std::thread(std::bind(&CDumpTransmit::DumpFileTransmitThread, this));
		return true;
	}
	return false;
}

void CDumpTransmit::Destroy() {
	m_bDumpFileTransmitThreadState = false;
	if (m_dumpFileTransmitThread.joinable()) {
		m_cvForTransmitRequestQueue.notify_all();
		m_dumpFileTransmitThread.join();
	}
}

void CDumpTransmit::AddNewDumpTransmitQueueData(const std::string& sProgramName, const std::string& sDumpFilePath, const std::string& sDumpFileName) {
	std::unique_lock<std::mutex> lck(m_csForTransmitRequestQueue);
	m_dumpTransmitRequestQueue.Push(new FDumpTransmitRequestQueueData(sProgramName.c_str(), (sDumpFilePath + "\\" + sDumpFileName).c_str(), sDumpFileName.c_str()));
	m_cvForTransmitRequestQueue.notify_all();
}

bool CDumpTransmit::HasDumpFile(const std::string& sDumpFilePath, const DWORD iPID, std::string& sOutDumpFileName) {
	WIN32_FIND_DATAA fileFindData;
	HANDLE hFileFindHandle = FindFirstFileA((sDumpFilePath + "\\*.dmp").c_str(), &fileFindData);
	char sPrefix[MAX_FILE_NAME];
	sprintf_s(sPrefix, MAX_FILE_NAME, "PID-%d", iPID);

	if (hFileFindHandle != INVALID_HANDLE_VALUE) {
		do {
			if (fileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			if (strncmp(fileFindData.cFileName, sPrefix, strlen(sPrefix)) == 0) {
				sOutDumpFileName = fileFindData.cFileName;
				FindClose(hFileFindHandle);
				return true;
			}
		} while (FindNextFileA(hFileFindHandle, &fileFindData) != 0);
	}
	return false;
}

void CDumpTransmit::DumpFileTransmitThread() {
	std::vector<uint8_t> messageBuffer(MAX_MESSAGE_BUFFER_LENGTH, '\0');

	while (m_bDumpFileTransmitThreadState) {
		std::unique_lock<std::mutex> lck(m_csForTransmitRequestQueue);
		m_cvForTransmitRequestQueue.wait(lck, [&]() { return !m_dumpTransmitRequestQueue.IsEmpty() || !m_bDumpFileTransmitThreadState; });
		lck.unlock();

		FDumpTransmitRequestQueueData* pQueueData = nullptr;
		if (m_dumpTransmitRequestQueue.Pop(pQueueData)) {
			std::ifstream dumpFileStream(pQueueData->m_sDumpFilePath, std::ios::in | std::ios::binary);
			if (dumpFileStream.is_open()) {
				WatchDogPacket::DumpTransmitStatus transmitStatus = WatchDogPacket::DumpTransmitStatus::DumpTransmitStatus_Start;

				while (!dumpFileStream.eof()) {
					std::this_thread::sleep_for(std::chrono::milliseconds(125));

					ZeroMemory(&messageBuffer.at(0), MAX_MESSAGE_BUFFER_LENGTH);
					std::streamsize iReadBytes = dumpFileStream.read(reinterpret_cast<char*>(&messageBuffer.at(0)), MAX_MESSAGE_BUFFER_LENGTH).gcount();
					if (iReadBytes < MAX_MESSAGE_BUFFER_LENGTH)
						transmitStatus = WatchDogPacket::DumpTransmitStatus::DumpTransmitStatus_End;
					else
						transmitStatus = WatchDogPacket::DumpTransmitStatus::DumpTransmitStatus_Loop;

					m_dumpTransmitSocket.Write(UTIL::CreateWatchDogDumpTransmitPacket(m_flatbuffer, WatchDogPacket::PacketType_DumpFile, transmitStatus, pQueueData->m_sProgramName, pQueueData->m_sDumpFileName, messageBuffer));
				}
				dumpFileStream.close();
				DeleteFileA(pQueueData->m_sDumpFilePath);

				delete pQueueData;
			}
		}
	}
}