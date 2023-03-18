#include "UDPSocket.hpp"
#include "../Session/Server/ServerSession.hpp"

using namespace SERVER::NETWORK::PROTOCOL::UDP;
using namespace SERVER::NETWORK::PROTOCOL::UTIL::UDP;
using namespace SERVER::NETWORK::PROTOCOL::UTIL::SOCKET;

UDPIPSocket::UDPIPSocket() : BaseSocket(UTIL::SOCKET::EPROTOCOLTYPE::EPT_UDP) {
}

UDPIPSocket::~UDPIPSocket() {
}

bool UDPIPSocket::WriteTo(const SocketAddress& sendAddress, const char* const sSendData, const uint16_t iDataLength) {
	return false;
}

bool UDPIPSocket::ReadFrom(char* const sReceiveBuffer, uint16_t& iRecvBytes) {
	return false;
}


/* UTIL */

bool SendTo(const::SOCKET hSocket, const SocketAddress& sendAddress, const char* const sSendBuffer, const uint16_t iDataLength) {
	SERVER::NETWORK::SESSION::SERVERSESSION::OVERLAPPED_EX sendToOverlapped;
	DWORD iSendBytes;
	WSABUF wsaBuffer;
	wsaBuffer.buf = const_cast<char* const>(sSendBuffer);
	wsaBuffer.len = iDataLength;

	if (WSASendTo(hSocket, &wsaBuffer, 1, &iSendBytes, 0, &sendAddress, sendAddress.GetSize(), &sendToOverlapped.m_wsaOverlapped, nullptr) == SOCKET_ERROR) {
		int iWSAErrorResult = 0;
		if (GetIOResult(iWSAErrorResult)) {
			// log
			return false;
		}
	}
	return true;
}

bool ReceiveFrom(const::SOCKET hSocket, char* const sReceiveBuffer, uint16_t& iReceiveBytes, SERVER::NETWORK::SESSION::SERVERSESSION::OVERLAPPED_EX& receiveOverlapped) {

	return false;
}