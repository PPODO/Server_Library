#include "IOCP.hpp"
#include <Functions/Functions/Log/Log.h>

using namespace FUNCTIONS::LOG;

NETWORK::NETWORKMODEL::IOCP::CIOCP::CIOCP(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_ProtocolType(ProtocolType), m_hIOCP(INVALID_HANDLE_VALUE), m_bIsRunMainThread(TRUE) {
	try {
		if (WSAStartup(WINSOCK_VERSION, &m_WinSockData) == SOCKET_ERROR) {
			throw FUNCTIONS::EXCEPTION::bad_wsastart();
		}
	}
	catch (const FUNCTIONS::EXCEPTION::bad_wsastart& Exception) {
		CLog::WriteLog(Exception.what());
		std::abort();
	}
}

NETWORK::NETWORKMODEL::IOCP::CIOCP::~CIOCP() {
	for (auto& It : m_WorkerThread) {
		PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
	}
	for (auto& It : m_WorkerThread) {
		if (It.joinable()) {
			It.join();
		}
	}
	
	InterlockedExchange16(&m_bIsRunMainThread, FALSE);

	WSACleanup();
}

bool NETWORK::NETWORKMODEL::IOCP::CIOCP::Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) {
	if (!InitializeHandles()) {
		return false;
	}
	
	if (!InitializeSession(BindAddress)) {
		return false;
	}

	CreateWorkerThread();

	return true;
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::Run() {
	while (m_bIsRunMainThread) {

	}
}

NETWORK::NETWORKMODEL::IOCP::CIOCP ASD() {
	return NETWORK::NETWORKMODEL::IOCP::CIOCP(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP);
}

bool NETWORK::NETWORKMODEL::IOCP::CIOCP::InitializeHandles() {
	if (!(m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))) {
		CLog::WriteLog(L"Initialize IOCP Handle : Failed To ~~");
		return false;
	}

	return true;
}

bool NETWORK::NETWORKMODEL::IOCP::CIOCP::InitializeSession(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) {
	using namespace SESSION::SERVERSESSION;
	using namespace UTIL::BASESOCKET;

	try {
		if (m_Listener = std::make_unique<CServerSession>(m_ProtocolType); !m_Listener->Initialize(BindAddress)) {
			return false;
		}

		if (m_ProtocolType & EPROTOCOLTYPE::EPT_TCP) {
			for (size_t i = 0; i < MAX_CLIENT_COUNT; i++) {
				if (std::unique_ptr<CServerSession> Client = std::make_unique<CServerSession>(EPROTOCOLTYPE::EPT_TCP); Client->Initialize(*m_Listener)) {
					m_Clients.push_back(std::move(Client));
					continue;
				}
				return false;
			}
		}
	}
	catch (const std::bad_alloc& Exception) {
		CLog::WriteLog(Exception.what());
		return false;
	}

	CLog::WriteLog(L"");
	return true;
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::CreateWorkerThread() {
	const size_t WorkerThreadCount = std::thread::hardware_concurrency() * 2;
	for (size_t i = 0; i < WorkerThreadCount; i++) {
		m_WorkerThread.push_back(std::thread(&NETWORK::NETWORKMODEL::IOCP::CIOCP::WorkerThread, this));
	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::WorkerThread() {
	using namespace UTIL::SESSION::SERVERSESSION::DETAIL;

	while (true) {
		DWORD RecvBytes = 0;
		void* CompletionKey = nullptr;
		OVERLAPPED_EX* OverlappedEx = nullptr;

		bool Succeed = GetQueuedCompletionStatus(0, &RecvBytes, reinterpret_cast<PULONG_PTR>(&CompletionKey), reinterpret_cast<LPOVERLAPPED*>(&OverlappedEx), INFINITE);

		if (!CompletionKey) {
			// Á¾·á µÊ.
			break;
		}

		if (OverlappedEx) {
			if (!Succeed || (Succeed && RecvBytes <= 0)) {
				switch (OverlappedEx->m_IOType) {
				case EIOTYPE::EIT_ACCEPT:

					break;
				default:

					break;
				}
				continue;
			}

			switch (OverlappedEx->m_IOType) {
			case EIOTYPE::EIT_DISCONNECT:

				break;
			case EIOTYPE::EIT_READ:

				break;
			case EIOTYPE::EIT_WRITE:

				break;
			}
		}
	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIOAccept(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	if (Session) {

		// Read;
		Session;
	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIOTryDisconnect(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	if (Session && Session->SocketRecycle()) {
		//
	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIODisconnected(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	if (Session && Session->Initialize(*m_Listener)) {
		
	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIOWrite(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	if (Session) {

	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIOReceive(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session, DWORD ReceiveBytes) {
	if (Session) {

	}
}