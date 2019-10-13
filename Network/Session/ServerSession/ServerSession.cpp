#include "ServerSession.h"

using namespace NETWORK::SOCKET::TCPIP;
using namespace NETWORK::SOCKET::UDPIP;

NETWORK::SESSION::SERVERSESSION::CServerSession::CServerSession(const UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) :
  m_AcceptOverlapped(UTIL::SESSION::SERVERSESSION::DETAIL::EIOTYPE::EIT_ACCEPT, this)
, m_DisconnectOverlapped(UTIL::SESSION::SERVERSESSION::DETAIL::EIOTYPE::EIT_DISCONNECT, this)
, m_SendOverlapped(UTIL::SESSION::SERVERSESSION::DETAIL::EIOTYPE::EIT_WRITE, this)
, m_ReceiveOverlapped(UTIL::SESSION::SERVERSESSION::DETAIL::EIOTYPE::EIT_READ, this) {

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
	if (!m_TCPSocket->Accept(*ServerSession.m_TCPSocket, m_AcceptOverlapped)) {
		return false;
	}
	return true;
}

bool NETWORK::SESSION::SERVERSESSION::CServerSession::SocketRecycle() {
	if (!m_TCPSocket->SocketRecycling(m_DisconnectOverlapped)) {
		return false;
	}
	return true;
}
