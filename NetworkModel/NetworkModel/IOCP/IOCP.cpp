#include "IOCP.hpp"
#include <Functions/Functions/Log/Log.hpp>
#include <MSWSock.h>
#pragma comment(lib, "mswsock.lib")

using namespace FUNCTIONS::LOG;

NETWORKMODEL::IOCP::CIOCP::CIOCP(const NETWORKMODEL::DETAIL::PACKETPROCESSORLIST& ProcessorList, const int PacketProcessLoopCount) : NETWORKMODEL::DETAIL::CNetworkModel(PacketProcessLoopCount, ProcessorList), m_hIOCP(INVALID_HANDLE_VALUE) {
}

NETWORKMODEL::IOCP::CIOCP::~CIOCP() {
}

bool NETWORKMODEL::IOCP::CIOCP::Initialize(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) {
	NETWORKMODEL::DETAIL::CNetworkModel::Initialize(ProtocolType, BindAddress);

	// 1. Initialize Handle;
	{
		if (!(m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))) {
			FUNCTIONS::LOG::CLog::WriteLog(L"Initialize IOCP Handle : Failed To Initialization IOCP Handle");
			return false;
		}
	}

	// 2. Initialize Session;
	{
		using namespace NETWORK::SESSION::SERVERSESSION;
		using namespace NETWORK::UTIL::BASESOCKET;

		try {
			if (m_Listener = new CServerSession(ProtocolType); !m_Listener->Initialize(BindAddress) || !m_Listener->RegisterIOCompletionPort(m_hIOCP)) {
				return false;
			}

			if ((ProtocolType & EPROTOCOLTYPE::EPT_UDP) && !m_Listener->ReceiveFrom()) {
				return false;
			}
			if (ProtocolType & EPROTOCOLTYPE::EPT_TCP) {
				for (size_t i = 0; i < MAX_CLIENT_COUNT; i++) {
					if (auto Session = new CServerSession(EPROTOCOLTYPE::EPT_TCP); Session->Initialize(*m_Listener)) {
						m_Clients.push_back(new DETAIL::CONNECTION(Session));
						continue;
					}
					return false;
				}
			}
		}
		catch (const std::bad_alloc & Exception) {
			CLog::WriteLog(Exception.what());
			return false;
		}

		CLog::WriteLog(L"Initialize IOCP Session : Session Initialization Succeeded!");
	}

	// 3. Create Worker Threads;
	{
		const size_t WorkerThreadCount = std::thread::hardware_concurrency() * 2;
		for (size_t i = 0; i < WorkerThreadCount; i++) {
			m_WorkerThread.push_back(std::thread(&NETWORKMODEL::IOCP::CIOCP::WorkerThread, this));
		}

		FUNCTIONS::LOG::CLog::WriteLog(L"Initialize IOCP Session : Worker Thread Creation Succeeded! - %d", WorkerThreadCount);
	}
	return true;
}

void NETWORKMODEL::IOCP::CIOCP::Run() {
	NETWORKMODEL::DETAIL::CNetworkModel::Run();
}

void NETWORKMODEL::IOCP::CIOCP::Destroy() {
	for (auto& It : m_WorkerThread) {
		PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
	}
	for (auto& It : m_WorkerThread) {
		if (It.joinable()) {
			It.join();
		}
	}
	m_WorkerThread.clear();

	m_ClientListLock.Lock();
	for (auto It : m_Clients) {
		if (It && It->m_Session) {
			delete It->m_Session;
		}
	}
	m_Clients.clear();
	m_ClientListLock.UnLock();

	if (m_Listener) {
		delete m_Listener;
	}
}

void NETWORKMODEL::IOCP::CIOCP::WorkerThread() {
	using namespace NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL;

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
			if (!Succeed || RecvBytes <= 0) {
				switch (OverlappedEx->m_IOType) {
				case EIOTYPE::EIT_ACCEPT:
					OnIOAccept(OverlappedEx);
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
				OnIOWrite(OverlappedEx->m_Owner, reinterpret_cast<const uint16_t&>(RecvBytes));
				break;
			case EIOTYPE::EIT_READFROM:
				OnIOReceiveFrom(OverlappedEx, reinterpret_cast<const uint16_t&>(RecvBytes));
				break;
			}
		}
	}
}

NETWORKMODEL::IOCP::DETAIL::CONNECTION* NETWORKMODEL::IOCP::CIOCP::OnIOAccept(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const AcceptExOverlappedEx) {
	using namespace FUNCTIONS::SOCKADDR;
	
	if (auto Session = AcceptExOverlappedEx->m_Owner) {
		sockaddr* LocalAddr = nullptr, *RemoteAddr = nullptr;
		int LocalLength = 0, RemoteLength = 0;
		GetAcceptExSockaddrs(AcceptExOverlappedEx->m_SocketMessage, 0, CSocketAddress::GetSize() + 16, CSocketAddress::GetSize() + 16, &LocalAddr, &LocalLength, &RemoteAddr, &RemoteLength);
		
		if (auto RemoteAddr_in = reinterpret_cast<sockaddr_in*>(RemoteAddr)) {
			if (auto Connection = GetConnectionFromListOrNull(Session)) {
				Connection->m_PeerInformation.m_RemoteAddress = FUNCTIONS::SOCKADDR::CSocketAddress(*RemoteAddr_in);

				if (Session->RegisterIOCompletionPort(m_hIOCP) && Session->Receive()) {
					CLog::WriteLog(L"Accept New Client!");
					return Connection;
				}
			}
		}
		CLog::WriteLog(L"Accept Failed!");
		Session->SocketRecycle();
	}
	return nullptr;
}

void NETWORKMODEL::IOCP::CIOCP::OnIOTryDisconnect(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	if (Session->SocketRecycle()) {
		CLog::WriteLog(L"Try Disconnect Client!");
	}
	else {
		OnIODisconnected(Session);
	}
}

NETWORKMODEL::IOCP::DETAIL::CONNECTION* NETWORKMODEL::IOCP::CIOCP::OnIODisconnected(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	// TO DO : Connection Remove From Client List
	if (auto Connection = GetConnectionFromListOrNull(Session)) {
		if (Session->Initialize(*m_Listener)) {
			CLog::WriteLog(L"Disconnect Client!");
			return Connection;
		}
	}
	return nullptr;
}

NETWORKMODEL::IOCP::DETAIL::CONNECTION* NETWORKMODEL::IOCP::CIOCP::OnIOWrite(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session, const uint16_t& SendBytes) {
	Session->SendCompletion(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP, SendBytes);
	return GetConnectionFromListOrNull(Session);
}

NETWORKMODEL::IOCP::DETAIL::CONNECTION* NETWORKMODEL::IOCP::CIOCP::OnIOReceive(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveOverlappedEx, const uint16_t& RecvBytes) {
	if (auto Connection = GetConnectionFromListOrNull(ReceiveOverlappedEx->m_Owner)) {

		ReceiveOverlappedEx->m_RemainReceivedBytes += RecvBytes;
		PacketForwardingLoop(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP, ReceiveOverlappedEx->m_SocketMessage, ReceiveOverlappedEx->m_RemainReceivedBytes, ReceiveOverlappedEx->m_LastReceivedPacketNumber, Connection);
		if (!Connection->m_Session->Receive()) {
			Connection->m_Session->SocketRecycle();
		}
		return Connection;
	}
	return nullptr;
}

NETWORKMODEL::IOCP::DETAIL::CONNECTION* NETWORKMODEL::IOCP::CIOCP::OnIOReceiveFrom(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveFromOverlappedEx, const uint16_t& RecvBytes) {
	if (auto Connection = GetConnectionFromListOrNull(ReceiveFromOverlappedEx->m_RemoteAddress)) {
		ReceiveFromOverlappedEx->m_RemainReceivedBytes += RecvBytes;
		ReceiveFromOverlappedEx->m_LastReceivedPacketNumber = Connection->m_PeerInformation.m_LastPacketNumber;

		if (NETWORK::UTIL::UDPIP::CheckAck(*ReceiveFromOverlappedEx)) {
			PacketForwardingLoop(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP, ReceiveFromOverlappedEx->m_SocketMessage, ReceiveFromOverlappedEx->m_RemainReceivedBytes, ReceiveFromOverlappedEx->m_LastReceivedPacketNumber, Connection);
			Connection->m_PeerInformation.m_LastPacketNumber = ReceiveFromOverlappedEx->m_LastReceivedPacketNumber;
		}

		if (!ReceiveFromOverlappedEx->m_Owner->ReceiveFrom()) {
			ReceiveFromOverlappedEx->m_Owner->ReceiveFrom();
		}
		return Connection;
	}
	return nullptr;
}