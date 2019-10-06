#include "IOCP.hpp"
#include <mutex>

using namespace NETWORK;
using namespace FUNCTIONS::LOG;

NETWORK::NETWORKMODEL::IOCP::CIOCP::CIOCP(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_ProtocolType(ProtocolType), m_hIOCP(INVALID_HANDLE_VALUE), m_hWaitForInitializedThread(INVALID_HANDLE_VALUE) {
	if (WSAStartup(WINSOCK_VERSION, &m_WinSockData) == SOCKET_ERROR) {
		CLog::WriteLog(L"");
		std::abort();
	}
}

NETWORK::NETWORKMODEL::IOCP::CIOCP::~CIOCP() {
	m_ClientSessions.clear();

	for (size_t i = 0; i < m_WorkerThreads.size(); i++) {
		PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
	}
	for (auto& Iterator : m_WorkerThreads) {
		if (Iterator.joinable()) {
			Iterator.join();
		}
	}

	if (m_hWaitForInitializedThread || m_hWaitForInitializedThread != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hWaitForInitializedThread);
		m_hWaitForInitializedThread = INVALID_HANDLE_VALUE;
	}

	if (m_hIOCP || m_hIOCP != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hIOCP);
		m_hIOCP = INVALID_HANDLE_VALUE;
	}
	WSACleanup();
}

bool NETWORK::NETWORKMODEL::IOCP::CIOCP::InitializeHandle() {
	if ((m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0)) == NULL) {
		CLog::WriteLog(L"Initialize IOCP Handle : Failed To Initialize IOCP Handle!");
		return false;
	}

	if ((m_hWaitForInitializedThread = CreateEvent(nullptr, false, false, nullptr)) == NULL) {
		CLog::WriteLog(L"Initialize IOCP Handle : Failed To Wait For Initialize Handle!");
		return false;
	}
	return true;
}

bool NETWORK::NETWORKMODEL::IOCP::CIOCP::InitializeWorkerThread() {
	size_t NumberOfProcessor = std::thread::hardware_concurrency() * 2;
	for (size_t i = 0; i < NumberOfProcessor; i++) {
		m_WorkerThreads.push_back(std::thread(&CIOCP::ProcessWorkerThread, this));
	}
	return true;
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::ProcessWorkerThread() {
	using namespace UTIL::NETWORKSESSION::SERVERSESSION::DETAIL;

	DWORD RecvBytes = 0;
	LPOVERLAPPED Overlapped = nullptr;
	void* CompletionKey = nullptr;

	while (true) {
		bool Succeed = GetQueuedCompletionStatus(m_hIOCP, &RecvBytes, reinterpret_cast<PULONG_PTR>(&CompletionKey), &Overlapped, INFINITE);

		if (!CompletionKey) {
			CLog::WriteLog(L"Shutdown!");
			return;
		}

		OVERLAPPED_EX* OverlappedEx = reinterpret_cast<OVERLAPPED_EX*>(Overlapped);

		if (OverlappedEx) {
			// 비정상적인 종료, 정상적인 종료 or 클라이언트 연결
			if (!Succeed || (Succeed && RecvBytes == 0)) {
				if (OverlappedEx->m_IOType == EIOTYPE::EIT_ACCEPT) {
					OnIOAccept(OverlappedEx->m_Owner);
				}
				else if(OverlappedEx->m_IOType == EIOTYPE::EIT_DISCONNECT) {
					OnIODisconnect(OverlappedEx->m_Owner);
				}
				else {
					OnIOTryDisconnect(OverlappedEx->m_Owner);
				}
				continue;
			}

			switch (OverlappedEx->m_IOType) {
			case EIOTYPE::EIT_READ:
				OnIORead(OverlappedEx->m_Owner, RecvBytes);
				break;
			case EIOTYPE::EIT_WRITE:

				break;
			}
		}
	}
}

bool NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIOAccept(SESSION::NETWORKSESSION::SERVERSESSION::CServerSession* const Owner) {
	if (Owner && UTIL::IOCP::RegisterIOCompletionPort(UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP, m_hIOCP, *Owner) && Owner->Read()) {
		CLog::WriteLog(L"Accept!");
		return true;
	}
	return false;
}

bool NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIOTryDisconnect(SESSION::NETWORKSESSION::SERVERSESSION::CServerSession* const Owner) {
	if (Owner && Owner->SocketRecycling()) {
		CLog::WriteLog(L"Try Disconnect!");
		return true;
	}
	return false;
}

bool NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIODisconnect(SESSION::NETWORKSESSION::SERVERSESSION::CServerSession* const Owner) {
	if (Owner && Owner->Initialize(*m_ServerSession)) {
		CLog::WriteLog(L"Disconnect is Successful!");
		return true;
	}
	return false;
}

bool NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIORead(SESSION::NETWORKSESSION::SERVERSESSION::CServerSession* const Owner, const DWORD& RecvBytes) {
	if (Owner) {
		if (Owner->GetReceivedData(UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP, RecvBytes)) {
			CLog::WriteLog(L"Read!");
			
			return Owner->Read();
		}
	}
	return false;
}

bool NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIOReadFrom(SESSION::NETWORKSESSION::SERVERSESSION::CServerSession* const Owner, const DWORD& RecvBytes) {
	if (Owner) {
		if (Owner->GetReceivedData(UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP, RecvBytes)) {
			CLog::WriteLog(L"Read From!");

			return Owner->ReadFrom();
		}
	}
	return false;
}