#include "User_Server.hpp"

using namespace SERVER::NETWORK::USER_SESSION::USER_SERVER;

User_Server::User_Server(NETWORK::PROTOCOL::UTIL::BSD_SOCKET::EPROTOCOLTYPE protocolType) : User(protocolType) {
}

bool User_Server::Initialize(FUNCTIONS::SOCKETADDRESS::SocketAddress& toAddress) {
	using namespace SERVER::NETWORK::PROTOCOL::UTIL;

	if (!User::Initialize(toAddress))
		return false;
	
	bool bResult = true;
	if (m_protocolType == EPROTOCOLTYPE::EPT_TCP || m_protocolType == EPROTOCOLTYPE::EPT_BOTH)
		bResult = m_pTCPSocekt->Listen();

	return bResult;
}

bool User_Server::Initialize(const User_Server& server) {
	return m_pTCPSocekt->Accept(*server.m_pTCPSocekt, m_acceptOverlapped);
}

bool User_Server::RegisterIOCompletionPort(const HANDLE& hIOCP) {
	if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_pTCPSocekt->GetSocket()), hIOCP, reinterpret_cast<ULONG_PTR>(this), 0) &&
		!CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_pUDPSocket->GetSocket()), hIOCP, reinterpret_cast<ULONG_PTR>(this), 0))
		if (WSAGetLastError() != 87)
			return false;

	return true;
}

bool User_Server::Receive() {
	return m_pTCPSocekt->Read(m_receiveOverlapped);
}

bool User_Server::ReceiveFrom() {
	return true;
}

bool User_Server::Send(char* const sSendData, const uint16_t iDataLength) {
	return m_pTCPSocekt->Write(sSendData, iDataLength, m_sendOverlapped);
}

bool User_Server::SendTo(const FUNCTIONS::SOCKETADDRESS::SocketAddress& sendAddress, char* const sSendData, const uint16_t iDataLength) {
	return false;
}
