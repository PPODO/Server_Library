#include "User_Server.hpp"

using namespace SERVER::NETWORK::USER_SESSION::USER_SERVER;

User_Server::User_Server(NETWORK::PROTOCOL::UTIL::BSD_SOCKET::EPROTOCOLTYPE protocolType) : User(protocolType),
																							m_acceptOverlapped(EIOTYPE::EIT_ACCEPT, this), 
																							m_disconnectOverlapped(EIOTYPE::EIT_DISCONNECT, this),
																							m_receiveOverlapped(EIOTYPE::EIT_READ, this), 
																							m_receiveFromOverlapped(EIOTYPE::EIT_READFROM, this), 
																							m_sendOverlapped(EIOTYPE::EIT_WRITE, this)
{}

bool User_Server::Initialize(FUNCTIONS::SOCKETADDRESS::SocketAddress& toAddress) {
	using namespace SERVER::NETWORK::PROTOCOL::UTIL;

	if (!User::Bind(toAddress))
		return false;
	
	bool bResult = true;
	if (m_protocolType & EPROTOCOLTYPE::EPT_TCP)
		bResult = m_pTCPSocekt->Listen();

	return bResult;
}

bool User_Server::Initialize(const User_Server& server) {
	return m_pTCPSocekt->Accept(*server.m_pTCPSocekt, m_acceptOverlapped);
}

bool User_Server::RegisterIOCompletionPort(const HANDLE& hIOCP) {
	if (m_pTCPSocekt && !CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_pTCPSocekt->GetSocket()), hIOCP, reinterpret_cast<ULONG_PTR>(this), 0)) {
		if (WSAGetLastError() != 87)
			return false;
	}

	if (m_pUDPSocket && !CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_pUDPSocket->GetSocket()), hIOCP, reinterpret_cast<ULONG_PTR>(this), 0)) {
		if (WSAGetLastError() != 87)
			return false;
	}

	return true;
}

bool User_Server::Receive() {
	return m_pTCPSocekt->Read(m_receiveOverlapped);
}

bool User_Server::ReceiveFrom() {
	return m_pUDPSocket->ReadFrom(m_receiveFromOverlapped);
}

bool User_Server::Send(char* const sSendData, const uint16_t iDataLength) {
	return m_pTCPSocekt->Write(sSendData, iDataLength, m_sendOverlapped);
}

bool User_Server::SendTo(const FUNCTIONS::SOCKETADDRESS::SocketAddress& sendAddress, char* const sSendData, const uint16_t iDataLength) {
	return m_pUDPSocket->WriteTo(sendAddress, sSendData, iDataLength);
}

bool User_Server::Send(const PACKET::PACKET_STRUCT& sendPacketStructure) {
	return m_pTCPSocekt->Write(sendPacketStructure, m_sendOverlapped);
}

bool User_Server::SendTo(const PeerInfo& peerInformation, PACKET::PACKET_STRUCT& sendPacketStructure) {
	sendPacketStructure.m_packetInfo.m_iPacketNumber = peerInformation.m_iLastPacketNumber;
	return m_pUDPSocket->WriteToReliable(peerInformation.m_remoteAddress, sendPacketStructure);
}

bool User_Server::SocketRecycle() {
	return m_pTCPSocekt->SocketRecycling(m_disconnectOverlapped);
}
