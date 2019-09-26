#include "PacketSession.h"

using namespace NETWORK::SESSION::PACKETSESSION;

CPacketSession::CPacketSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_NetworkSession(std::make_shared<NETWORK::SESSION::NETWORKSESSION::CNetworkSession>(ProtocolType)) {
}

CPacketSession::~CPacketSession() {
}