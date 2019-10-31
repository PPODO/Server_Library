#include "IOCP.hpp"
#include <Functions/Functions/Log/Log.h>

using namespace FUNCTIONS::LOG;

NETWORKMODEL::IOCP::CIOCP::CIOCP(const DETAIL::PACKETPROCESSORLIST& ProcessorList) : DETAIL::CNetworkModel(ProcessorList), m_hIOCP(INVALID_HANDLE_VALUE), m_bIsRunMainThread(TRUE) {
	m_Command.AddNewAction("shutdown", std::bind(&NETWORKMODEL::IOCP::CIOCP::Destroy, this));
}

NETWORKMODEL::IOCP::CIOCP::~CIOCP() {
}

bool NETWORKMODEL::IOCP::CIOCP::Initialize(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) {
	if (!InitializeHandles()) {
		return false;
	}
	
	if (!InitializeSession(ProtocolType, BindAddress)) {
		return false;
	}

	CreateWorkerThread();
	return true;
}

void NETWORKMODEL::IOCP::CIOCP::Run() {
	while (m_bIsRunMainThread) {
		if (auto PacketData(GetPacketDataFromQueue()); PacketData) {
			if (auto Processor(GetProcessorFromList(PacketData->m_PacketStructure.m_PacketInformation.m_PacketType)); Processor) {
				Processor(PacketData);
			}
		}
	}
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

	InterlockedExchange16(&m_bIsRunMainThread, FALSE);

	m_Clients.clear();

	m_Command.Shutdown();
}

bool NETWORKMODEL::IOCP::CIOCP::InitializeSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) {
	using namespace NETWORK::SESSION::SERVERSESSION;
	using namespace NETWORK::UTIL::BASESOCKET;

	try {
		if (m_Listener = std::make_unique<CServerSession>(ProtocolType); !m_Listener->Initialize(BindAddress) || !m_Listener->RegisterIOCompletionPort(m_hIOCP)) {
			return false;
		}

		if ((ProtocolType & EPROTOCOLTYPE::EPT_UDP) && !m_Listener->ReceiveFrom()) {
			return false;
		}
		if (ProtocolType & EPROTOCOLTYPE::EPT_TCP) {
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

void NETWORKMODEL::IOCP::CIOCP::OnIOAccept(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	if (Session) {
		if (Session->RegisterIOCompletionPort(m_hIOCP) && Session->Receive()) {
			CLog::WriteLog(L"Accept New Client!");
			return;
		}
		CLog::WriteLog(L"Accept Failed!");
		Session->SocketRecycle();
	}
}

void NETWORKMODEL::IOCP::CIOCP::OnIOTryDisconnect(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	if (Session) {
		if (Session->SocketRecycle()) {
			CLog::WriteLog(L"Try Disconnect Client!");
		}
		else {
			OnIODisconnected(Session);
		}
	}
}

void NETWORKMODEL::IOCP::CIOCP::OnIODisconnected(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	if (Session && Session->Initialize(*m_Listener)) {
		CLog::WriteLog(L"Disconnect Client!");
	}
}

void NETWORKMODEL::IOCP::CIOCP::OnIOWrite(NETWORK::SESSION::SERVERSESSION::CServerSession* const Session) {
	if (Session && Session->SendCompletion(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP)) {
		CLog::WriteLog(L"Send!");
	}
}

void NETWORKMODEL::IOCP::CIOCP::OnIOReceive(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveOverlappedEx, const uint16_t& RecvBytes) {
	if (ReceiveOverlappedEx) {
		ReceiveOverlappedEx->m_RemainReceivedBytes += RecvBytes;
		PacketForwardingLoop(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP, ReceiveOverlappedEx->m_Owner, ReceiveOverlappedEx->m_SocketMessage, ReceiveOverlappedEx->m_RemainReceivedBytes, ReceiveOverlappedEx->m_LastReceivedPacketNumber);
		if (!ReceiveOverlappedEx->m_Owner->Receive()) {
			ReceiveOverlappedEx->m_Owner->SocketRecycle();
		}
	}
}

void NETWORKMODEL::IOCP::CIOCP::OnIOReceiveFrom(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX* const ReceiveFromOverlappedEx, const uint16_t& RecvBytes) {
	if (NETWORK::SESSION::SERVERSESSION::CServerSession* Owner = ReceiveFromOverlappedEx->m_Owner; Owner) {
		ReceiveFromOverlappedEx->m_RemainReceivedBytes += RecvBytes;

		if (NETWORK::UTIL::UDPIP::CheckAck(*ReceiveFromOverlappedEx)) {
			ReceiveFromOverlappedEx->m_LastReceivedPacketNumber = GetPeerInformation(ReceiveFromOverlappedEx->m_RemoteAddress).m_LastPacketNumber;
			PacketForwardingLoop(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP, ReceiveFromOverlappedEx->m_Owner, ReceiveFromOverlappedEx->m_SocketMessage, ReceiveFromOverlappedEx->m_RemainReceivedBytes, ReceiveFromOverlappedEx->m_LastReceivedPacketNumber);
			UpdatePeerInformation(ReceiveFromOverlappedEx->m_RemoteAddress, ReceiveFromOverlappedEx->m_LastReceivedPacketNumber);
		}
		if (!Owner->ReceiveFrom()) {
			Owner->SocketRecycle();
		}
	}
}