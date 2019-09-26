#include "ClientSession.h"

using namespace NETWORK::SESSION::CLIENTSESSION;

CClientSession::CClientSession(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : PACKETSESSION::CPacketSession(ProtocolType) {
}

CClientSession::~CClientSession() {
}