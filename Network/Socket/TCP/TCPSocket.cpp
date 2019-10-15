#include "TCPSocket.h"
#include <Functions/Functions/Log/Log.h>
#include <MSWSock.h>
#pragma comment(lib, "mswsock.lib")

using namespace NETWORK::SOCKET::BASESOCKET;
using namespace NETWORK::SOCKET::TCPIP;
using namespace FUNCTIONS::LOG;

CTCPIPSocket::CTCPIPSocket() : CBaseSocket(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP) {
	LINGER OptionVar;
	OptionVar.l_onoff = true;
	OptionVar.l_linger = 0;

	try {
		if (NETWORK::UTIL::BASESOCKET::SetSockOption(GetSocket(), SOL_SOCKET, SO_LINGER, &OptionVar, sizeof(LINGER)) == NETWORK::ERRORCODE::ENETRESULT::ENETSOCKTOPFAIL) {
			throw FUNCTIONS::EXCEPTION::bad_sockopt();
		}
	}
	catch (FUNCTIONS::EXCEPTION::bad_sockopt & Exception) {
		CLog::WriteLog(Exception.what());
	}
}

CTCPIPSocket::~CTCPIPSocket() {
}

bool CTCPIPSocket::Listen(const int32_t& BackLogCount) {
	if (listen(GetSocket(), BackLogCount) == SOCKET_ERROR) {
		CLog::WriteLog(L"Listen : Failed To Listen Process! - %d", WSAGetLastError());
		return false;
	}
	return true;
}

bool CTCPIPSocket::Connect(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress) {
	if (WSAConnect(GetSocket(), &ConnectAddress, ConnectAddress.GetSize(), nullptr, nullptr, 0, 0) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Connect : Failed To Connect To Server! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool CTCPIPSocket::Accept(const CTCPIPSocket& ListenSocket, NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& AcceptOverlapped) {
	int Flag = 0, Size = sizeof(int);
	if (getsockopt(GetSocket(), SOL_SOCKET, SO_ACCEPTCONN, reinterpret_cast<char*>(&Flag), &Size) == SOCKET_ERROR) {
		CLog::WriteLog(L"");
		return false;
	}

	size_t AddrLen = FUNCTIONS::SOCKADDR::CSocketAddress::GetSize() + 16;
	// AcceptEx의 네번 째 매개변수에는 0이 들어가야함. 그렇지 않으면 Accept와 동시의 데이터를 받겠다는 뜻이 되기 때문에 어떠한 데이터가 들어오기 전 까지는 블럭됨.
	if (!AcceptEx(ListenSocket.GetSocket(), GetSocket(), GetReceiveBufferPtr(), 0, AddrLen, AddrLen, nullptr, &AcceptOverlapped.m_Overlapped)) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"Accept : Failed To Accept - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool CTCPIPSocket::Write(const char* const SendData, const size_t& DataLength, NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped) {
	return UTIL::TCPIP::Send(GetSocket(), SendData, DataLength, SendOverlapped);
}

bool CTCPIPSocket::Read(char* const ReadBuffer, size_t&& ReadedSize, NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& RecvOverlapped) {
	if (UTIL::TCPIP::Receive(GetSocket(), GetReceiveBufferPtr(), ReadedSize, RecvOverlapped)) {
		if (ReadBuffer) {
			CopyReceiveBuffer(ReadBuffer, ReadedSize);
		}
		return true;
	}
	return false;
}

bool CTCPIPSocket::SocketRecycling(NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& DisconnectOverlapped) {
	shutdown(GetSocket(), SD_BOTH);
	if (!TransmitFile(GetSocket(), NULL, 0, 0, &DisconnectOverlapped.m_Overlapped, nullptr, TF_DISCONNECT | TF_REUSE_SOCKET)) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			CLog::WriteLog(L"Socket Recycling Work Failure! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

// UTIL


inline bool NETWORK::UTIL::TCPIP::Send(const::SOCKET& Socket, const char* const SendBuffer, const size_t& SendBufferSize, NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped) {
	DWORD SendBytes = 0;
	WSABUF Buffer;
	Buffer.buf = const_cast<char* const>(SendBuffer);
	Buffer.len = SendBufferSize;

	if (WSASend(Socket, &Buffer, 1, &SendBytes, 0, &SendOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"WSA Send : Failed To WSA Send! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

inline bool NETWORK::UTIL::TCPIP::Receive(const::SOCKET& Socket, char* const ReceiveBuffer, size_t& ReceiveBufferSize, NETWORK::UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& const RecvOverlapped) {
	DWORD RecvBytes = 0, Flag = 0;

	RecvOverlapped.m_WSABuffer.buf = ReceiveBuffer + RecvOverlapped.m_RemainReceivedBytes;
	RecvOverlapped.m_WSABuffer.len = SOCKET::BASESOCKET::MAX_RECEIVE_BUFFER_SIZE;
	RecvOverlapped.m_SocketMessage = RecvOverlapped.m_WSABuffer.buf - (RecvOverlapped.m_RemainReceivedBytes < 0 ? 0 : RecvOverlapped.m_RemainReceivedBytes);

	if (WSARecv(Socket, &RecvOverlapped.m_WSABuffer, 1, &RecvBytes, &Flag, &RecvOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			FUNCTIONS::LOG::CLog::WriteLog(L"WSA Recv : Failed To WSA Recv! - %d", WSAGetLastError());
			return false;
		}
	}
	ReceiveBufferSize = RecvBytes;

	return true;
}