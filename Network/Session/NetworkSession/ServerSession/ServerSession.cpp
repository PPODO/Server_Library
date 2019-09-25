#include "ServerSession.h"
#include <Network/Session/NetworkSession/ClientSession/ClientSession.h>

using namespace NETWORK::SESSION::SERVERSESSION;

CServerSession::CServerSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_NetworkSession(std::make_shared<NETWORK::SESSION::NETWORKSESSION::CNetworkSession>(ProtocolType)) {
}

CServerSession::~CServerSession() {
}