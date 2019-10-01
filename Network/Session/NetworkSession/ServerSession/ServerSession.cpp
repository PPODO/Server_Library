#include "ServerSession.h"

using namespace NETWORK::UTIL::NETWORKSESSION::SERVERSESSION::DETAIL;
using namespace NETWORK::SESSION::NETWORKSESSION::SERVERSESSION;

CServerSession::CServerSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_NetworkSession(ProtocolType) {
	m_DisconnectOverlapped.m_IOType = EIOTYPE::EIT_DISCONNECT;
	m_AcceptOverlapped.m_IOType = EIOTYPE::EIT_ACCEPT;
	m_RecvOverlapped.m_IOType = EIOTYPE::EIT_READ;
	m_SendOverlapped.m_IOType = EIOTYPE::EIT_WRITE;

	m_DisconnectOverlapped.m_Owner = m_AcceptOverlapped.m_Owner = m_RecvOverlapped.m_Owner = m_SendOverlapped.m_Owner = this;
}

CServerSession::~CServerSession() {
}