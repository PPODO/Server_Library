#include "UDPSocket.hpp"
#include "../UserSession/Server/User_Server.hpp"

using namespace SERVER::NETWORK::PROTOCOL::UDP;
using namespace SERVER::NETWORK::PROTOCOL::UTIL::UDP;
using namespace SERVER::NETWORK::PROTOCOL::UTIL::BSD_SOCKET;
using namespace SERVER::FUNCTIONS::LOG;

UDPIPSocket::UDPIPSocket() : BaseSocket(UTIL::BSD_SOCKET::EPROTOCOLTYPE::EPT_UDP) {
}

UDPIPSocket::~UDPIPSocket() {
}

bool UDPIPSocket::WriteTo(const SocketAddress& sendAddress, const char* const sSendData, const uint16_t iDataLength) {
	return UTIL::UDP::SendTo(GetSocket(), sendAddress, sSendData, iDataLength);
}

bool UDPIPSocket::WriteTo(const FUNCTIONS::SOCKETADDRESS::SocketAddress& sendAddress, const NETWORK::PACKET::PACKET_STRUCT& sendPacketStructure) {


}

bool UDPIPSocket::ReadFrom(char* const sReceiveBuffer, uint16_t& iRecvBytes) {
	NETWORK::USER_SESSION::USER_SERVER::OVERLAPPED_EX receiveOverlapped;
	return UTIL::UDP::ReceiveFrom(GetSocket(), sReceiveBuffer, iRecvBytes, receiveOverlapped);
}

bool UDPIPSocket::ReadFrom(NETWORK::USER_SESSION::USER_SERVER::OVERLAPPED_EX& receiveOverlapped) {
	uint16_t iRecvBytes = 0;
	return UTIL::UDP::ReceiveFrom(GetSocket(), GetReceiveBuffer(), iRecvBytes, receiveOverlapped);
}


/* UTIL */

bool SERVER::NETWORK::PROTOCOL::UTIL::UDP::SendTo(const::SOCKET hSocket, const SocketAddress& sendAddress, const char* const sSendBuffer, const uint16_t iDataLength) {
	SERVER::NETWORK::USER_SESSION::USER_SERVER::OVERLAPPED_EX sendToOverlapped;
	DWORD iSendBytes;
	WSABUF wsaBuffer;
	wsaBuffer.buf = const_cast<char* const>(sSendBuffer);
	wsaBuffer.len = iDataLength;

	if (WSASendTo(hSocket, &wsaBuffer, 1, &iSendBytes, 0, &sendAddress, sendAddress.GetSize(), &sendToOverlapped.m_wsaOverlapped, nullptr) == SOCKET_ERROR) {
		int iWSALastErrorCode = GetWSAErrorResult({ WSA_IO_PENDING, WSAEWOULDBLOCK });
		if (iWSALastErrorCode != 0) {
			Log::WriteLog(L"WSA SendTo : Failed to WSASend! - %d", iWSALastErrorCode);
			return false;
		}
	}
	return true;
}

bool SERVER::NETWORK::PROTOCOL::UTIL::UDP::ReceiveFrom(const::SOCKET hSocket, char* const sReceiveBuffer, uint16_t& iReceiveBytes, SERVER::NETWORK::USER_SESSION::USER_SERVER::OVERLAPPED_EX& receiveOverlapped) {
	DWORD iRecvBytes = 0, iFlag = 0;
	sockaddr* pRemoteAddress = const_cast<sockaddr*>(&receiveOverlapped.m_remoteAddress);
	INT iAddressSize = SocketAddress::GetSize();

	receiveOverlapped.m_pReceiveBuffer = sReceiveBuffer;

	receiveOverlapped.m_wsaBuffer.buf = sReceiveBuffer + receiveOverlapped.m_iRemainReceiveBytes;
	receiveOverlapped.m_wsaBuffer.len = SERVER::NETWORK::PROTOCOL::BSD_SOCKET::MAX_RECEIVE_BUFFER_SIZE - receiveOverlapped.m_iRemainReceiveBytes;

	if (WSARecvFrom(hSocket, &receiveOverlapped.m_wsaBuffer, 1, &iRecvBytes, &iFlag, pRemoteAddress, &iAddressSize, &receiveOverlapped.m_wsaOverlapped, nullptr)) {
		int iWSALastErrorCode = GetWSAErrorResult({ WSA_IO_PENDING, WSAEWOULDBLOCK });
		if (iWSALastErrorCode != 0) {
			Log::WriteLog(L"WSA RecvFrom : Failed to WSASend! - %d", iWSALastErrorCode);
			return false;
		}
	}
	return false;
}