#include "ClientSession.h"

using namespace NETWORK::SESSION::NETWORKSESSION::CLIENTSESSION;

CClientSession::CClientSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_Session(ProtocolType) {
}

CClientSession::~CClientSession() {
}