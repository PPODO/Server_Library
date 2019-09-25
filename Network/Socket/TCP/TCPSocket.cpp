#include "TCPSocket.h"
#include <Functions/Functions/Log/Log.h>
#include <MSWSock.h>
#pragma comment(lib, "mswsock.lib")

using namespace NETWORK::SOCKET::BASESOCKET;
using namespace NETWORK::SOCKET::TCPIP;
using namespace FUNCTIONS::LOG;

CTCPIPSocket::CTCPIPSocket() : CBaseSocket(UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP) {
	LINGER OptionVar;
	OptionVar.l_onoff = true;
	OptionVar.l_linger = 10;

	try {
		if (setsockopt(GetSocketHandle(), SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&OptionVar), sizeof(LINGER)) == SOCKET_ERROR) {
			throw "Failed To Setting Linger Option! %d";
		}
	}
	catch (const char* const Exception) {
		CLog::WriteLog(Exception, WSAGetLastError());
	}
}

CTCPIPSocket::~CTCPIPSocket() {
}

bool CTCPIPSocket::Listen(const size_t BackLogCount) {
	if (listen(GetSocketHandle(), BackLogCount) == SOCKET_ERROR) {
		CLog::WriteLog(L"Listen : Failed To Listen Process! - %d", WSAGetLastError());
		return false;
	}
	return true;
}

bool CTCPIPSocket::Connect(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress) {
	if (WSAConnect(GetSocketHandle(), &ConnectAddress, ConnectAddress.GetSize(), nullptr, nullptr, 0, 0) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Connect : Failed To Connect To Server! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool CTCPIPSocket::Accept(std::shared_ptr<CTCPIPSocket> ListenSocket, NETWORK::UTIL::BASESOCKET::OVERLAPPED_EX& AcceptOverlapped) {
	int Flag = 0, Size = sizeof(int);
	if (getsockopt(GetSocketHandle(), SOL_SOCKET, SO_ACCEPTCONN, reinterpret_cast<char*>(&Flag), &Size) == SOCKET_ERROR) {
		CLog::WriteLog(L"");
		return false;
	}
	else if (Flag == 0) {
		size_t AddrLen = FUNCTIONS::SOCKADDR::CSocketAddress::GetSize() + 16;

		if (!AcceptEx(ListenSocket->GetSocketHandle(), GetSocketHandle(), GetReceiveBufferPtr(), BASESOCKET::MAX_RECEIVE_BUFFER_SIZE, AddrLen, AddrLen, nullptr, &AcceptOverlapped.m_Overlapped)) {
			if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
				CLog::WriteLog(L"Accept : Failed To Accept - %d", WSAGetLastError());
				return false;
			}
		}
		return true;
	}
	return false;
}

bool CTCPIPSocket::WriteProcess(const char* const SendData, const size_t& DataLength, WSAOVERLAPPED* const SendOverlapped) {
	DWORD SendBytes = 0;
	WSABUF SendBuffer;
	SendBuffer.buf = const_cast<char* const>(SendData);
	SendBuffer.len = DataLength;

	if (WSASend(GetSocketHandle(), &SendBuffer, 1, &SendBytes, 0, SendOverlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"WSA Send : Failed To WSA Send! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool CTCPIPSocket::ReadProcess(char* const ReadBuffer, size_t& ReadedSize, WSAOVERLAPPED* const RecvOverlapped) {
	DWORD RecvBytes = 0, Flag = 0;
	WSABUF RecvBuffer;
	RecvBuffer.buf = ReadBuffer;
	RecvBuffer.len;

	if (WSARecv(GetSocketHandle(), &RecvBuffer, 1, &RecvBytes, &Flag, RecvOverlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"WSA Recv : Failed To WSA Recv! - %d", WSAGetLastError());
			return false;
		}
	}
	return false;
}