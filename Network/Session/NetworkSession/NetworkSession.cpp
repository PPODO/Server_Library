#include "NetworkSession.h"
#include <Functions/Functions/Log/Log.h>
#include <Network/Session/NetworkSession/ServerSession/ServerSession.h>

using namespace FUNCTIONS::LOG;
using namespace NETWORK::UTIL::BASESOCKET;
using namespace NETWORK::SESSION::NETWORKSESSION;

CNetworkSession::CNetworkSession(const EPROTOCOLTYPE& ProtocolType) : m_ProtocolType(ProtocolType) {
	try {
		if (ProtocolType & EPROTOCOLTYPE::EPT_TCP) {
			m_TCPSocket = std::make_unique<NETWORK::SOCKET::TCPIP::CTCPIPSocket>();
		}
		if (ProtocolType & EPROTOCOLTYPE::EPT_UDP) {
			m_UDPSocket = std::make_unique<NETWORK::SOCKET::UDPIP::CUDPIPSocket>();
		}
	}
	catch (const std::bad_alloc& Exception) {
		CLog::WriteLog(L"Initialize Network Session : %s!", Exception.what());
		std::abort();
	}
}

CNetworkSession::~CNetworkSession() {
}