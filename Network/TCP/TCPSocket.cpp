#include "TCPSocket.hpp"
#include "ServerSession.hpp"
#include <MSWSock.h>
#pragma comment(lib, "mswsock.lib")


using namespace SERVER::NETWORK::SOCKET::TCPIP;
using namespace SERVER::FUNCTIONS::LOG;

TCPIPSocket::TCPIPSocket() : BaseSocket(UTIL::SOCKET::EPROTOCOLTYPE::EPT_TCP) {

}

TCPIPSocket::~TCPIPSocket() {
}

bool TCPIPSocket::Listen(const int32_t iBackLogCount) {
	if (listen(GetSocket(), iBackLogCount) == SOCKET_ERROR) {
		Log::WriteLog(L"Listen : Failed To Listen Process! - %d", WSAGetLastError());
		return false;
	}
	return true;
}

bool TCPIPSocket::Connect(const FUNCTIONS::SOCKETADDRESS::SocketAddress& connectAddress) {
	if (WSAConnect(GetSocket(), &connectAddress, connectAddress.GetSize(), nullptr, nullptr, 0, 0) == SOCKET_ERROR) {
		Log::WriteLog(L"Connect : Failed To Connect To Server! - %d", WSAGetLastError());
		return false;
	}
	return true;
}

bool TCPIPSocket::Accept(const TCPIPSocket& listenSocket, SESSION::SERVERSESSION::OVERLAPPED_EX& acceptOverlapped) {
	int iFlag = 0, iSize = sizeof(int);
	if (getsockopt(GetSocket(), SOL_SOCKET, SO_ACCEPTCONN, reinterpret_cast<char*>(&iFlag), &iSize) == SOCKET_ERROR)
		return false;

	acceptOverlapped.m_sSocketMessage = GetReceiveBuffer();

	DWORD iAddrLen = FUNCTIONS::SOCKETADDRESS::SocketAddress::GetSize() + 16;
	if (!AcceptEx(listenSocket.GetSocket(), GetSocket(), GetReceiveBuffer(), 0, iAddrLen, iAddrLen, nullptr, &acceptOverlapped.m_wsaOverlapped)) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			Log::WriteLog(L"Accept : Failed To Accept! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

// TCP Send/Receive
bool SERVER::NETWORK::UTIL::TCPIP::Send(const::SOCKET& hSocket, char* const sSendBuffer, const uint16_t iSendBufferSize) {
	DWORD iSendBytes = 0;
	WSABUF wsaBuffer;
	wsaBuffer.buf = sSendBuffer;
	wsaBuffer.len = iSendBufferSize;

	if (WSASend(hSocket, &wsaBuffer, 1, &iSendBytes, 0, nullptr, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			Log::WriteLog(L"WSA Send : Failed to WSASend! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool SERVER::NETWORK::UTIL::TCPIP::Receive(const::SOCKET& hSocket, char* const sReceiveBuffer, uint16_t& iReceiveBufferSize) {
	DWORD iReceiveBytes = 0, iFlag = 0;
	WSABUF wsaBuffer;
	wsaBuffer.buf = sReceiveBuffer;
	wsaBuffer.len = NETWORK::SOCKET::MAX_RECEIVE_BUFFER_SIZE;

	if (WSARecv(hSocket, &wsaBuffer, 1, &iReceiveBytes, &iFlag, nullptr, nullptr)) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			Log::WriteLog(L"WSA Recv : Failed To WSARecv! - %d", WSAGetLastError());
			return false;
		}
	}

	iReceiveBufferSize = iReceiveBytes;
	return true;
}