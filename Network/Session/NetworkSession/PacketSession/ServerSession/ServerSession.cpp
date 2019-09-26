#include "ServerSession.h"
#include <Network/Session/NetworkSession/PacketSession/ClientSession/ClientSession.h>

using namespace NETWORK::SESSION::SERVERSESSION;

CServerSession::CServerSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : PACKETSESSION::CPacketSession(ProtocolType) {
}

CServerSession::~CServerSession() {
}