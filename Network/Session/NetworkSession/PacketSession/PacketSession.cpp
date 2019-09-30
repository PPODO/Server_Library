#include "PacketSession.h"

using namespace NETWORK::SESSION::PACKETSESSION;

CPacketSession::CPacketSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_NetworkSession(std::make_unique<NETWORK::SESSION::NETWORKSESSION::CNetworkSession>(ProtocolType)) {
	ZeroMemory(m_PacketBuffer, MAX_PACKET_BUFFER_SIZE);
}

CPacketSession::~CPacketSession() {
}