#include "ClientSession.h"

using namespace NETWORK::SESSION::CLIENTSESSION;

CClientSession::CClientSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_Session(ProtocolType) {
}

CClientSession::~CClientSession() {
}