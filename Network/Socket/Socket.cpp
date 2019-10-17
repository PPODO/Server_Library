#include "Socket.h"
#include <Functions/Functions/Log/Log.h>
#include <MSWSock.h>
#pragma comment(lib, "mswsock.lib")

using namespace NETWORK::SOCKET::BASESOCKET;
using namespace FUNCTIONS::LOG;

CBaseSocket::CBaseSocket(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType) : m_Socket(UTIL::BASESOCKET::CreateSocketByProtocolType(ProtocolType)) {
	ZeroMemory(m_ReceiveMessageBuffer, MAX_BUFFER_LENGTH);
}

CBaseSocket::~CBaseSocket() {
	Destroy();
}

bool CBaseSocket::Bind(const FUNCTIONS::SOCKADDR::CSocketAddress& BindAddress) {
	if (bind(m_Socket, &BindAddress, BindAddress.GetSize()) == SOCKET_ERROR) {
		CLog::WriteLog("Bind Failed");
		return false;
	}
	return true;
}

void CBaseSocket::Destroy() {
	if (m_Socket != INVALID_SOCKET_VALUE) {
		shutdown(m_Socket, SD_BOTH);
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET_VALUE;
	}
}