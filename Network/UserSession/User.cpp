#include "User.hpp"

using namespace SERVER::NETWORK::USER_SESSION;
using namespace SERVER::FUNCTIONS::LOG;

User::User(const EPROTOCOLTYPE protocolType) : m_protocolType(protocolType) {
	using namespace SERVER::NETWORK::PROTOCOL::UTIL::BSD_SOCKET;

	try {
		if(m_protocolType & EPROTOCOLTYPE::EPT_TCP)
			m_pTCPSocekt = std::make_unique<TCPIPSocket>();
		if(m_protocolType & EPROTOCOLTYPE::EPT_UDP)
			m_pUDPSocket = std::make_unique<UDPIPSocket>();
	}
	catch (std::bad_alloc& exception) {
		Log::WriteLog(FUNCTIONS::UTIL::MBToUni(exception.what()).c_str());
		std::abort();
	}
}

User::~User() {
}

bool User::Bind(FUNCTIONS::SOCKETADDRESS::SocketAddress& toAddress) {
	if (m_pTCPSocekt)
		if(!m_pTCPSocekt->Bind(toAddress)) return false;
	if (m_pUDPSocket)
		if(!m_pUDPSocket->Bind(toAddress)) return false;

	return true;
}

bool User::Connect(FUNCTIONS::SOCKETADDRESS::SocketAddress& toAddress) {
	if (!m_pTCPSocekt || !m_pTCPSocekt->Connect(toAddress))
		return false;

	return true;
}

bool User::Receive(char* const sReceiveBuffer, uint16_t& iReceiveBytes) {
	if (m_pTCPSocekt)
		return m_pTCPSocekt->Read(sReceiveBuffer, iReceiveBytes);
	return false;
}

bool User::ReceiveFrom(char* const sReceiveBuffer, uint16_t& iReceiveBytes) {
	if (m_pUDPSocket)
		return m_pUDPSocket->ReadFrom(sReceiveBuffer, iReceiveBytes);
	return false;
}

bool User::Send(char* const sSendData, const uint16_t iDataLength) {
	if (m_pTCPSocekt)
		return m_pTCPSocekt->Write(sSendData, iDataLength);
	return false;
}

bool User::SendTo(const FUNCTIONS::SOCKETADDRESS::SocketAddress& sendAddress, char* const sSendData, const uint16_t iDataLength) {
	if (m_pUDPSocket)
		m_pUDPSocket->WriteTo(sendAddress, sSendData, iDataLength);
	return false;
}
