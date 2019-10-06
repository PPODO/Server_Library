#include "ServerSession.h"

using namespace NETWORK::UTIL::NETWORKSESSION::SERVERSESSION::DETAIL;
using namespace NETWORK::SESSION::NETWORKSESSION::SERVERSESSION;

CServerSession::CServerSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_Session(ProtocolType) 
, m_AcceptOverlapped(EIOTYPE::EIT_ACCEPT, this), m_DisconnectOverlapped(EIOTYPE::EIT_DISCONNECT, this), m_SendOverlapped(EIOTYPE::EIT_WRITE, this), m_RecvOverlapped(EIOTYPE::EIT_READ, this)
, m_SendToOverlapped(EIOTYPE::EIT_WRITETO, this), m_RecvFromOverlapped(EIOTYPE::EIT_READFROM, this) {

}

CServerSession::~CServerSession() {
}

bool NETWORK::SESSION::NETWORKSESSION::SERVERSESSION::CServerSession::GetReceivedData(const UTIL::BASESOCKET::EPROTOCOLTYPE& Protocol, const DWORD& RecvBytes) {
	FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock(m_GetReceivedDataLock);

	if (const FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* Data = m_Session.GetReceivedPacket(Protocol, RecvBytes)) {

		return true;
	}
	return false;
}