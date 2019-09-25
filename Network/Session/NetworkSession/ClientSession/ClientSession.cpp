#include "ClientSession.h"

using namespace NETWORK::SESSION::CLIENTSESSION;

CClientSession::CClientSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_NetworkSession(std::make_shared<NETWORK::SESSION::NETWORKSESSION::CNetworkSession>(ProtocolType)) {
}

CClientSession::~CClientSession() {
}