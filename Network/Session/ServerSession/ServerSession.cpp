#include "ServerSession.h"

using namespace NETWORK::SOCKET::TCPIP;
using namespace NETWORK::SOCKET::UDPIP;

NETWORK::SESSION::SERVERSESSION::CServerSession::CServerSession(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) :
  m_AcceptOverlapped(UTIL::SESSION::SERVERSESSION::DETAIL::EIOTYPE::EIT_ACCEPT, this)
, m_DisconnectOverlapped(UTIL::SESSION::SERVERSESSION::DETAIL::EIOTYPE::EIT_DISCONNECT, this)
, m_SendOverlapped(UTIL::SESSION::SERVERSESSION::DETAIL::EIOTYPE::EIT_WRITE, this)
, m_ReceiveOverlapped(UTIL::SESSION::SERVERSESSION::DETAIL::EIOTYPE::EIT_READ, this)
, m_ReceiveFromOverlapped(UTIL::SESSION::SERVERSESSION::DETAIL::EIOTYPE::EIT_READFROM, this) {

	try {
		if (ProtocolType & UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP) {
			if (m_TCPSocket = std::make_unique<CTCPIPSocket>(); !m_TCPSocket) {
				throw FUNCTIONS::EXCEPTION::bad_tcpalloc();
			}
		}
		if (ProtocolType & UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP) {
			if (m_UDPSocket = std::make_unique<CUDPIPSocket>(); !m_UDPSocket) {
				throw FUNCTIONS::EXCEPTION::bad_udpalloc();
			}
		}
	}
	catch (const std::exception& Exception) {
		FUNCTIONS::LOG::CLog::WriteLog(Exception.what());
		std::abort();
	}
}

NETWORK::SESSION::SERVERSESSION::CServerSession::~CServerSession() {
}

bool NETWORK::SESSION::SERVERSESSION::CServerSession::Initialize(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress, const int32_t& BackLogCount) {
	if (m_TCPSocket && (!m_TCPSocket->Bind(BindAddress) || !m_TCPSocket->Listen(BackLogCount))) {
		return false;
	}
	if (m_UDPSocket && !m_UDPSocket->Bind(BindAddress)) {
		return false;
	}
	return true;
}

bool NETWORK::SESSION::SERVERSESSION::CServerSession::Initialize(const CServerSession& ServerSession) {
	if (m_TCPSocket && !m_TCPSocket->Accept(*ServerSession.m_TCPSocket, m_AcceptOverlapped)) {
		return false;
	}
	return true;
}

bool NETWORK::SESSION::SERVERSESSION::CServerSession::SocketRecycle() {
	if (m_TCPSocket && !m_TCPSocket->SocketRecycling(m_DisconnectOverlapped)) {
		return false;
	}
	return true;
}

inline bool NETWORK::SESSION::SERVERSESSION::CServerSession::SendCompletion(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) {
	if (ProtocolType == UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP) {
		return m_TCPSocket->SendCompletion();
	}
	else if (ProtocolType == UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP) {
		return m_UDPSocket->SendCompletion();
	}
	return false;
}

bool NETWORK::SESSION::SERVERSESSION::CServerSession::RegisterIOCompletionPort(const HANDLE& hIOCP) {
	if (m_TCPSocket && !CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_TCPSocket->GetSocket()), hIOCP, reinterpret_cast<ULONG_PTR>(this), 0)) {
		return false;
	}
	if (m_UDPSocket && !CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_UDPSocket->GetSocket()), hIOCP, reinterpret_cast<ULONG_PTR>(this), 0)) {
		return false;
	}
	return true;
}

void NETWORK::SESSION::SERVERSESSION::CServerSession::UpdatePeerInformation(const FUNCTIONS::SOCKADDR::CSocketAddress& PeerAddress, const uint16_t& UpdatedPacketNumber) {
	FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_ConnectionListLock);

	if (const auto& Iterator = std::find_if(m_ConnectedPeers.begin(), m_ConnectedPeers.end(), [&PeerAddress](const NETWORK::SOCKET::UDPIP::PEERINFO& Address) -> bool { if (PeerAddress == Address.m_RemoteAddress) { return true; } return false; }); Iterator != m_ConnectedPeers.end()) {
		Iterator->m_LastPacketNumber = UpdatedPacketNumber;
	}
	else {
		FUNCTIONS::LOG::CLog::WriteLog(L"Add New Peer!");
		m_ConnectedPeers.emplace_back(PeerAddress, UpdatedPacketNumber);
	}
}

NETWORK::SOCKET::UDPIP::PEERINFO NETWORK::SESSION::SERVERSESSION::CServerSession::GetPeerInformation(const FUNCTIONS::SOCKADDR::CSocketAddress& PeerAddress) {
	FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_ConnectionListLock);

	if (const auto& Iterator = std::find_if(m_ConnectedPeers.begin(), m_ConnectedPeers.end(), [&PeerAddress](const NETWORK::SOCKET::UDPIP::PEERINFO& Address) -> bool { if (PeerAddress == Address.m_RemoteAddress) { return true; } return false; }); Iterator != m_ConnectedPeers.end()) {
		return *Iterator;
	}
	else {
		FUNCTIONS::LOG::CLog::WriteLog(L"Add New Peer!");
		return m_ConnectedPeers.emplace_back(PeerAddress, 0);
	}
}

bool NETWORK::UTIL::UDPIP::CheckAck(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& Overlapped) {
	if (NETWORK::SESSION::SERVERSESSION::CServerSession* Owner = Overlapped.m_Owner; Overlapped.m_SocketMessage) {
		const int32_t ReadedValue = *reinterpret_cast<const int16_t*>(Overlapped.m_SocketMessage);
		const int16_t AckValue = static_cast<int16_t>(ReadedValue);
		const int16_t PacketNumber = static_cast<int16_t>((ReadedValue >> 16));

		if (AckValue == 9999) {
			Overlapped.m_LastReceivedPacketNumber = PacketNumber;
			return Owner->SendCompletion(UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP);
		}
		else if (AckValue == 0) {
			int16_t AckNumber = 9999;
			int16_t PacketNumber = Overlapped.m_LastReceivedPacketNumber + 1;
			int32_t Result = ((PacketNumber << 16) | AckNumber);
			Overlapped.m_RemainReceivedBytes -= sizeof(ReadedValue);
			MoveMemory(Overlapped.m_SocketMessage, Overlapped.m_SocketMessage + sizeof(ReadedValue), Overlapped.m_RemainReceivedBytes);
			return Owner->SendTo(Overlapped.m_RemoteAddress, reinterpret_cast<const char* const>(&Result), sizeof(int32_t));
		}
	}
	return false;
}
