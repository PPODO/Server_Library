#include "ServerSession.h"
#include <Network/Session/NetworkSession/PacketSession/ClientSession/ClientSession.h>

using namespace NETWORK::UTIL::BASESOCKET;
using namespace NETWORK::SESSION::SERVERSESSION;

NETWORK::SESSION::SERVERSESSION::CServerSession::CServerSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_Session(ProtocolType) {
	m_DisconnectOverlapped.m_IOType = EIOTYPE::EIT_DISCONNECT;
	m_AcceptOverlapped.m_IOType = EIOTYPE::EIT_ACCEPT;
	m_RecvOverlapped.m_IOType = EIOTYPE::EIT_READ;
	m_SendOverlapped.m_IOType = EIOTYPE::EIT_WRITE;

	m_DisconnectOverlapped.m_Owner = m_AcceptOverlapped.m_Owner = m_RecvOverlapped.m_Owner = m_SendOverlapped.m_Owner = this;
}

CServerSession::~CServerSession() {
}
