#include "ServerSession.hpp"

using namespace NETWORK::SOCKET::TCPIP;
using namespace NETWORK::SOCKET::UDPIP;

NETWORK::SESSION::SERVERSESSION::CServerSession::CServerSession(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_TCPSocket(nullptr), m_UDPSocket(nullptr),
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

NETWORK::SESSION::SERVERSESSION::CServerSession::CServerSession(CServerSession&& rvalue) : m_TCPSocket(std::move(rvalue.m_TCPSocket)), m_UDPSocket(std::move(rvalue.m_UDPSocket))
, m_AcceptOverlapped(rvalue.m_AcceptOverlapped)
, m_DisconnectOverlapped(rvalue.m_DisconnectOverlapped)
, m_SendOverlapped(rvalue.m_SendOverlapped)
, m_ReceiveOverlapped(rvalue.m_ReceiveOverlapped)
, m_ReceiveFromOverlapped(rvalue.m_ReceiveFromOverlapped) {
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
		if (WSAGetLastError() != 87) {
			return false;
		}
	}
	if (m_UDPSocket && !CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_UDPSocket->GetSocket()), hIOCP, reinterpret_cast<ULONG_PTR>(this), 0)) {
		if (WSAGetLastError() != 87) {
			return false;
		}
	}
	return true;
}

bool NETWORK::UTIL::UDPIP::CheckAck(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& Overlapped) {
	if (NETWORK::SESSION::SERVERSESSION::CServerSession* Owner = Overlapped.m_Owner; Overlapped.m_SocketMessage) {
		const int32_t ReadedValue = *reinterpret_cast<const int32_t*>(Overlapped.m_SocketMessage);
		const int16_t AckValue = static_cast<int16_t>(ReadedValue);
		const int16_t PacketNumber = static_cast<int16_t>((ReadedValue >> 16));

		// TODO PacketNumber가 이전에 받은 Number보다 작을때 따로 처리해주는 로직이 필요
		if (AckValue == 9999) {
			Overlapped.m_LastReceivedPacketNumber = PacketNumber;
			Overlapped.m_RemainReceivedBytes -= sizeof(ReadedValue);
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
