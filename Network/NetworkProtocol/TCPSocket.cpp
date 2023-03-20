#include "TCPSocket.hpp"
#include "../UserSession/Server/User_Server.hpp"
#include <MSWSock.h>
#pragma comment(lib, "mswsock.lib")


using namespace SERVER::NETWORK::PROTOCOL::TCP;
using namespace SERVER::FUNCTIONS::LOG;

TCPIPSocket::TCPIPSocket() : BaseSocket(UTIL::BSD_SOCKET::EPROTOCOLTYPE::EPT_TCP) {
	LINGER option;
	option.l_onoff = true;
	option.l_linger = 0;

	setsockopt(GetSocket(), SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&option), sizeof(LINGER));
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

bool TCPIPSocket::Accept(const TCPIPSocket& listenSocket, USER_SESSION::USER_SERVER::OVERLAPPED_EX& acceptOverlapped) {
	int iFlag = 0, iSize = sizeof(int);
	if (getsockopt(GetSocket(), SOL_SOCKET, SO_ACCEPTCONN, reinterpret_cast<char*>(&iFlag), &iSize) == SOCKET_ERROR)
		return false;

	acceptOverlapped.m_wsaBuffer.buf = GetReceiveBuffer();
	acceptOverlapped.m_wsaBuffer.len = BSD_SOCKET::MAX_RECEIVE_BUFFER_SIZE;

	DWORD iAddrLen = FUNCTIONS::SOCKETADDRESS::SocketAddress::GetSize() + 16;
	if (!AcceptEx(listenSocket.GetSocket(), GetSocket(), GetReceiveBuffer(), 0, iAddrLen, iAddrLen, nullptr, &acceptOverlapped.m_wsaOverlapped)) {
		int iWSALastErrorCode = UTIL::BSD_SOCKET::GetWSAErrorResult({ WSA_IO_PENDING, WSAEWOULDBLOCK });
		if (iWSALastErrorCode != 0) {
			Log::WriteLog(L"Accept : Failed To Accept! - %d", iWSALastErrorCode);
			return false;
		}
	}
	return true;
}

bool TCPIPSocket::Write(const char* const sSendData, const uint16_t iDataLength) {
	USER_SESSION::USER_SERVER::OVERLAPPED_EX sendOverlapped;

	return UTIL::TCP::Send(GetSocket(), const_cast<char* const>(sSendData), iDataLength, sendOverlapped);
}

bool TCPIPSocket::Write(const char* const sSendData, const uint16_t iDataLength, USER_SESSION::USER_SERVER::OVERLAPPED_EX& sendOverlapped) {
	if (auto pQueueResult = m_sendMessageQueue.Push(new WSASendData(sSendData, iDataLength)))
		return UTIL::TCP::Send(GetSocket(), pQueueResult->m_sBuffer, pQueueResult->m_iDataLength, sendOverlapped);

	return false;
}

bool TCPIPSocket::Read(char* const sReceiveBuffer, uint16_t& iReceiveBytes) {
	USER_SESSION::USER_SERVER::OVERLAPPED_EX receiveOverlapped;

	return UTIL::TCP::Receive(GetSocket(), sReceiveBuffer, iReceiveBytes, receiveOverlapped);
}

bool TCPIPSocket::Read(USER_SESSION::USER_SERVER::OVERLAPPED_EX& receiveOverlapped) {
	uint16_t iReceiveBytes = 0;
	return UTIL::TCP::Receive(GetSocket(), GetReceiveBuffer(), iReceiveBytes, receiveOverlapped);
}

bool TCPIPSocket::SocketRecycling(USER_SESSION::USER_SERVER::OVERLAPPED_EX& disconnectOverlapped) {
	shutdown(GetSocket(), SD_BOTH);
	if (TransmitFile(GetSocket(), NULL, 0, 0, &disconnectOverlapped.m_wsaOverlapped, NULL, TF_DISCONNECT | TF_REUSE_SOCKET))
		if (WSAGetLastError() != WSA_IO_PENDING)
			Log::WriteLog(L"Socket Recycling Work Failure! - %d", WSAGetLastError()); return false;

	return true;
}

// TCP Send/Receive
bool SERVER::NETWORK::PROTOCOL::UTIL::TCP::Send(const::SOCKET& hSocket, char* const sSendBuffer, const uint16_t iSendBufferSize, USER_SESSION::USER_SERVER::OVERLAPPED_EX& sendOverlapped) {
	DWORD iSendBytes = 0;
	WSABUF wsaBuffer;
	wsaBuffer.buf = sSendBuffer;
	wsaBuffer.len = iSendBufferSize;

	if (WSASend(hSocket, &wsaBuffer, 1, &iSendBytes, 0, &sendOverlapped.m_wsaOverlapped, nullptr) == SOCKET_ERROR) {
		int iWSALastErrorCode = UTIL::BSD_SOCKET::GetWSAErrorResult({ WSA_IO_PENDING, WSAEWOULDBLOCK });
		if (iWSALastErrorCode != 0) {
			Log::WriteLog(L"WSA Send : Failed to WSASend! - %d", iWSALastErrorCode);
			return false;
		}
	}
	return true;
}

bool SERVER::NETWORK::PROTOCOL::UTIL::TCP::Receive(const::SOCKET& hSocket, char* const sReceiveBuffer, uint16_t& iReceiveBufferSize, USER_SESSION::USER_SERVER::OVERLAPPED_EX& receiveOverlapped) {
	DWORD iReceiveBytes = 0, iFlag = 0;
	
	receiveOverlapped.m_wsaBuffer.buf = sReceiveBuffer + receiveOverlapped.m_iRemainReceiveBytes;
	receiveOverlapped.m_wsaBuffer.len = NETWORK::PROTOCOL::BSD_SOCKET::MAX_RECEIVE_BUFFER_SIZE;
	receiveOverlapped.m_sSocketMessage = receiveOverlapped.m_wsaBuffer.buf - receiveOverlapped.m_iRemainReceiveBytes;

	if (WSARecv(hSocket, &receiveOverlapped.m_wsaBuffer, 1, &iReceiveBytes, &iFlag, &receiveOverlapped.m_wsaOverlapped, nullptr) == SOCKET_ERROR) {
		int iWSALastErrorCode = UTIL::BSD_SOCKET::GetWSAErrorResult({ WSA_IO_PENDING, WSAEWOULDBLOCK });
		if (iWSALastErrorCode != 0) {
			Log::WriteLog(L"WSA Recv : Failed To WSARecv! - %d", WSAGetLastError());
			return false;
		}
	}

	iReceiveBufferSize = iReceiveBytes;
	return true;
}