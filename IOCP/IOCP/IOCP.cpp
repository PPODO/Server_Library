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
	m_Command.AddNewAction("shutdown", std::bind(&NETWORK::NETWORKMODEL::IOCP::CIOCP::Destroy, this));
}

NETWORK::NETWORKMODEL::IOCP::CIOCP::~CIOCP() {
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
		if (!m_Queue.IsEmpty()) {
			if (FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* Data; m_Queue.Pop(Data) && Data) {
				delete Data;
			}
		}
	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::Destroy() {
	for (auto& It : m_WorkerThread) {
		PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
	}
	for (auto& It : m_WorkerThread) {
		if (It.joinable()) {
			It.join();
		}
	}
	m_WorkerThread.clear();

	InterlockedExchange16(&m_bIsRunMainThread, FALSE);

	m_Clients.clear();

	m_Command.Shutdown();
}

bool NETWORK::NETWORKMODEL::IOCP::CIOCP::InitializeHandles() {
	if (!(m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))) {
		CLog::WriteLog(L"Initialize IOCP Handle : Failed To Initialization IOCP Handle");
		return false;
	}

	return true;
}

bool NETWORK::NETWORKMODEL::IOCP::CIOCP::InitializeSession(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) {
	using namespace SESSION::SERVERSESSION;
	using namespace UTIL::BASESOCKET;

	try {
		if (m_Listener = std::make_unique<CServerSession>(m_ProtocolType); !m_Listener->Initialize(BindAddress) || !m_Listener->RegisterIOCompletionPort(m_hIOCP)) {
			return false;
		}

		if ((m_ProtocolType & EPROTOCOLTYPE::EPT_UDP) && !m_Listener->ReceiveFrom()) {
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

	CLog::WriteLog(L"Initialize IOCP Session : Session Initialization Succeeded!");
	return true;
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::CreateWorkerThread() {
	const size_t WorkerThreadCount = std::thread::hardware_concurrency() * 2;
	for (size_t i = 0; i < WorkerThreadCount; i++) {
		m_WorkerThread.push_back(std::thread(&NETWORK::NETWORKMODEL::IOCP::CIOCP::WorkerThread, this));
	}

	CLog::WriteLog(L"Initialize IOCP Session : Worker Thread Creation Succeeded! - %d", WorkerThreadCount);
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::WorkerThread() {
	using namespace UTIL::SESSION::SERVERSESSION::DETAIL;

	while (true) {
		DWORD RecvBytes = 0;
		void* CompletionKey = nullptr;
		OVERLAPPED_EX* OverlappedEx = nullptr;

		bool Succeed = GetQueuedCompletionStatus(m_hIOCP, &RecvBytes, reinterpret_cast<PULONG_PTR>(&CompletionKey), reinterpret_cast<LPOVERLAPPED*>(&OverlappedEx), INFINITE);

		if (!CompletionKey) {
			// Á¾·á µÊ.
			break;
		}

		if (OverlappedEx) {
			if (!Succeed || (Succeed && RecvBytes <= 0)) {
				switch (OverlappedEx->m_IOType) {
				case EIOTYPE::EIT_ACCEPT:
					OnIOAccept(OverlappedEx->m_Owner);
					break;
				case EIOTYPE::EIT_DISCONNECT:
					OnIODisconnected(OverlappedEx->m_Owner);
					break;
				default:
					OnIOTryDisconnect(OverlappedEx->m_Owner);
					break;
				}
				continue;
			}

			switch (OverlappedEx->m_IOType) {
				break;
			case EIOTYPE::EIT_READ:
				OnIOReceive(OverlappedEx, reinterpret_cast<const uint16_t&>(RecvBytes));
				break;
			case EIOTYPE::EIT_WRITE:
				OnIOWrite(OverlappedEx->m_Owner);
				break;
			case EIOTYPE::EIT_READFROM:
				OnIOReceiveFrom(OverlappedEx, reinterpret_cast<const uint16_t&>(RecvBytes));
				break;
			}
		}
	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::PacketForwardingLoop(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveOverlappedEx) {
	using namespace NETWORK::PACKET;

	while (true) {
		PACKET_STRUCTURE PacketStructure;
		int16_t RemainBytes = ReceiveOverlappedEx->m_RemainReceivedBytes;
		int16_t LastReceivedPacketNumber = ReceiveOverlappedEx->m_LastReceivedPacketNumber;

		if (RemainBytes >= DETAIL::PACKET_INFORMATION::GetSize()) {
			PacketStructure.m_PacketInformation = *reinterpret_cast<DETAIL::PACKET_INFORMATION*>(ReceiveOverlappedEx->m_SocketMessage);
			RemainBytes -= DETAIL::PACKET_INFORMATION::GetSize();
		}

		if (RemainBytes >= PacketStructure.m_PacketInformation.m_PacketSize && PacketStructure.m_PacketInformation.m_PacketNumber == LastReceivedPacketNumber + 1) {
			uint16_t TotalBytes = (PacketStructure.m_PacketInformation.GetSize() + PacketStructure.m_PacketInformation.m_PacketSize);

			CopyMemory(PacketStructure.m_PacketData, ReceiveOverlappedEx->m_SocketMessage + PacketStructure.m_PacketInformation.GetSize(), PacketStructure.m_PacketInformation.m_PacketSize);

			FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* QueueData = new FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData(ReceiveOverlappedEx->m_Owner, PacketStructure);

			m_Queue.Push(QueueData);

			ReceiveOverlappedEx->m_LastReceivedPacketNumber = LastReceivedPacketNumber + 1;
			ReceiveOverlappedEx->m_RemainReceivedBytes -= TotalBytes;
			MoveMemory(ReceiveOverlappedEx->m_SocketMessage, ReceiveOverlappedEx->m_SocketMessage + TotalBytes, ReceiveOverlappedEx->m_RemainReceivedBytes);
		}
		else {
			break;
		}

	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIOAccept(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	if (Session) {
		if (Session->RegisterIOCompletionPort(m_hIOCP) && Session->Receive()) {
			CLog::WriteLog(L"Accept New Client!");
			return;
		}
		CLog::WriteLog(L"Accept Failed!");
		Session->SocketRecycle();
	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIOTryDisconnect(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	if (Session) {
		if (Session->SocketRecycle()) {
			CLog::WriteLog(L"Try Disconnect Client!");
		}
		else {
			OnIODisconnected(Session);
		}
	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIODisconnected(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	if (Session && Session->Initialize(*m_Listener)) {
		CLog::WriteLog(L"Disconnect Client!");
	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIOWrite(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	if (Session) {

	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIOReceive(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveOverlappedEx, const uint16_t& RecvBytes) {
	if (ReceiveOverlappedEx) {
		ReceiveOverlappedEx->m_RemainReceivedBytes += RecvBytes;

		PacketForwardingLoop(ReceiveOverlappedEx);
		if (!ReceiveOverlappedEx->m_Owner->Receive()) {
			ReceiveOverlappedEx->m_Owner->SocketRecycle();
		}
	}
}

void NETWORK::NETWORKMODEL::IOCP::CIOCP::OnIOReceiveFrom(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveFromOverlappedEx, const uint16_t& RecvBytes) {
	if (NETWORK::SESSION::SERVERSESSION::CServerSession* Owner = ReceiveFromOverlappedEx->m_Owner; Owner) {
		ReceiveFromOverlappedEx->m_RemainReceivedBytes += RecvBytes;

		if (UTIL::UDPIP::CheckAck(*ReceiveFromOverlappedEx)) {
			Owner->RegisterNewPeer(ReceiveFromOverlappedEx->m_RemoteAddress);
			PacketForwardingLoop(ReceiveFromOverlappedEx);
		}
		if (!Owner->ReceiveFrom()) {
			Owner->SocketRecycle();
		}
	}
}