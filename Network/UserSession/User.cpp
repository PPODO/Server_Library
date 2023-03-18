#include "User.hpp"
#include "../../Functions/Log/Log.hpp"

using namespace SERVER::NETWORK::USER_SESSION;
using namespace SERVER::FUNCTIONS::LOG;

User::User(const NETWORK::PROTOCOL::UTIL::SOCKET::EPROTOCOLTYPE protocolType) : m_protocolType(protocolType) {
	using namespace SERVER::NETWORK::PROTOCOL::UTIL::SOCKET;

	try {
		m_pTCPSocekt = std::make_unique<TCPIPSocket>();
		m_pUDPSocket = std::make_unique<UDPIPSocket>();
	}
	catch (std::bad_alloc& exception) {
		Log::WriteLog(FUNCTIONS::UTIL::MBToUni(exception.what()).c_str());
		std::abort();
	}
}

User::~User() {
}

bool User::Initialize(FUNCTIONS::SOCKETADDRESS::SocketAddress& toAddress) {
	if (!m_pTCPSocekt && !m_pTCPSocekt->Bind(toAddress))
		return false;
	if (!m_pUDPSocket && !m_pUDPSocket->Bind(toAddress))
		return false;

	return true;
}