#include "UDPSocket.h"
#include <Functions/Functions/Log/Log.h>
#include <Network/Session/ServerSession/ServerSession.h>

using namespace NETWORK::SOCKET::BASESOCKET;
using namespace NETWORK::SOCKET::UDPIP;
using namespace FUNCTIONS::LOG;

CUDPIPSocket::CUDPIPSocket() : BASESOCKET::CBaseSocket(UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP), m_ReliableThreadHandle(&CUDPIPSocket::ReliableThread, this), m_ThreadRunState(1) {
	try {
		m_hWaitForInitializeThreadEvent = CreateEvent(nullptr, false, false, nullptr);
		if (!m_hWaitForInitializeThreadEvent) {
			throw "";
		}

		m_hSendCompleteEvent = CreateEvent(nullptr, false, false, nullptr);
		if (!m_hSendCompleteEvent) {
			throw "";
		}

		m_hNewReliableDataEvent = CreateEvent(nullptr, false, false, nullptr);
		if (!m_hNewReliableDataEvent) {
			throw "";
		}

		WaitForSingleObject(m_hWaitForInitializeThreadEvent, INFINITE);
	}
	catch (const std::exception&) {
		std::abort();
	}
}

CUDPIPSocket::~CUDPIPSocket() {
	InterlockedExchange16(&m_ThreadRunState, 0);
	SetEvent(m_hNewReliableDataEvent);

	if (m_ReliableThreadHandle.joinable()) {
		m_ReliableThreadHandle.join();
	}

	if (m_hSendCompleteEvent) {
		CloseHandle(m_hSendCompleteEvent);
		m_hSendCompleteEvent = INVALID_HANDLE_VALUE;
	}
	
	if (m_hNewReliableDataEvent) {
		CloseHandle(m_hNewReliableDataEvent);
		m_hNewReliableDataEvent = INVALID_HANDLE_VALUE;
	}

	if (m_hWaitForInitializeThreadEvent) {
		CloseHandle(m_hWaitForInitializeThreadEvent);
		m_hWaitForInitializeThreadEvent = INVALID_HANDLE_VALUE;
	}
}

bool NETWORK::SOCKET::UDPIP::CUDPIPSocket::WriteToQueue(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, NETWORK::PACKET::PACKET_STRUCTURE& SendPacketStructure) {
	try {
		if (auto ReliableData = new FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::ReliableData(SendPacketStructure, SendAddress); m_ReliableDataQueue.Push(ReliableData)) {
			return SetEvent(m_hNewReliableDataEvent);
		}
	}
	catch (const std::bad_alloc& Exception) {
		CLog::WriteLog(Exception.what());
	}
	return false;
}

bool NETWORK::SOCKET::UDPIP::CUDPIPSocket::WriteTo(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const NETWORK::PACKET::PACKET_STRUCTURE& SendPacketStructure) {
	char TempBuffer[::MAX_RECEIVE_BUFFER_SIZE] = { "\0" };
	int16_t AckValue = 0;

	CopyMemory(TempBuffer, reinterpret_cast<const char* const>(&AckValue), sizeof(AckValue));
	CopyMemory(TempBuffer + sizeof(AckValue), reinterpret_cast<const char* const>(&SendPacketStructure.m_PacketInformation), SendPacketStructure.m_PacketInformation.GetSize());
	CopyMemory(TempBuffer + sizeof(AckValue) + SendPacketStructure.m_PacketInformation.GetSize(), SendPacketStructure.m_PacketData, SendPacketStructure.m_PacketInformation.m_PacketSize);
	uint16_t TotalSendBytes = sizeof(AckValue) + SendPacketStructure.m_PacketInformation.GetSize() + SendPacketStructure.m_PacketInformation.m_PacketSize;

	return UTIL::UDPIP::SendTo(GetSocket(), SendAddress, TempBuffer, TotalSendBytes);
}

bool NETWORK::SOCKET::UDPIP::CUDPIPSocket::WriteTo(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const char* const SendData, const uint16_t& SendDataLength) {
	return UTIL::UDPIP::SendTo(GetSocket(), SendAddress, SendData, SendDataLength);
}

bool NETWORK::SOCKET::UDPIP::CUDPIPSocket::ReadFrom(char* const ReceivedBuffer, uint16_t& RecvBytes) {
	if (ReceivedBuffer) {
		UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX Overlapped;
		if (UTIL::UDPIP::ReceiveFrom(GetSocket(), ReceivedBuffer, RecvBytes, Overlapped)) {

			return true;// UTIL::UDPIP::CheckAck();
		}
	}
	return false;
}

bool NETWORK::SOCKET::UDPIP::CUDPIPSocket::ReadFrom(UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& ReceiveOverlapped) {
	uint16_t RecvBytes = 0;
	return UTIL::UDPIP::ReceiveFrom(GetSocket(), GetReceiveBufferPtr(), RecvBytes, ReceiveOverlapped);
}

bool NETWORK::SOCKET::UDPIP::CUDPIPSocket::SendCompletion() {
	return SetEvent(m_hSendCompleteEvent);
}

void NETWORK::SOCKET::UDPIP::CUDPIPSocket::ReliableThread() {
	SetEvent(m_hWaitForInitializeThreadEvent);

	while (m_ThreadRunState) {
		if (!m_ReliableDataQueue.IsEmpty()) {
			if (WaitForSingleObject(m_hNewReliableDataEvent, INFINITE) != WAIT_OBJECT_0) {
				CLog::WriteLog(L"");
				continue;
			}
		}

		if (FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::ReliableData* QueueData; m_ReliableDataQueue.Pop(QueueData) && QueueData) {
			for (size_t i = 0; i < REPEAT_COUNT_FOR_RELIABLE_SEND; i++) {
				if (WriteTo(QueueData->m_SendAddress, QueueData->m_SendPacketStructure)) {
					if (WaitForSingleObject(m_hSendCompleteEvent, 10) == WAIT_OBJECT_0) {
						delete QueueData;
						break;
					}
				}
			}
		}
	}
}

bool NETWORK::UTIL::UDPIP::SendTo(const::SOCKET& Socket, const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const char* const SendBuffer, const uint16_t& SendBytes) {
	UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX SendOverlapped;
	WSABUF SendDataBuffer;
	SendDataBuffer.buf = const_cast<char* const>(SendBuffer);
	SendDataBuffer.len = SendBytes;

	if (WSASendTo(Socket, &SendDataBuffer, 1, reinterpret_cast<LPDWORD>(&const_cast<uint16_t&>(SendBytes)), 0, &SendAddress, SendAddress.GetSize(), &SendOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"WSA Send To : Failed To WSA Send To! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

bool NETWORK::UTIL::UDPIP::ReceiveFrom(const ::SOCKET& Socket, char* const ReceivedBuffer, uint16_t& ReceivedBytes, UTIL::SESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& ReceiveOverlapped) {
	DWORD RecvBytes = 0, Flag = 0;
	int32_t AddressSize = ReceiveOverlapped.m_RemoteAddress.GetSize();

	ReceiveOverlapped.m_WSABuffer.buf = ReceivedBuffer + ReceiveOverlapped.m_RemainReceivedBytes;
	ReceiveOverlapped.m_WSABuffer.len = ::MAX_RECEIVE_BUFFER_SIZE - ReceiveOverlapped.m_RemainReceivedBytes;
	ReceiveOverlapped.m_SocketMessage = ReceiveOverlapped.m_WSABuffer.buf - ReceiveOverlapped.m_RemainReceivedBytes;

	if (WSARecvFrom(Socket, &ReceiveOverlapped.m_WSABuffer, 1, &RecvBytes, &Flag, const_cast<sockaddr*>(&ReceiveOverlapped.m_RemoteAddress), &AddressSize, &ReceiveOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != EWOULDBLOCK) {
			FUNCTIONS::LOG::CLog::WriteLog(L"WSA Recv From : Failed To WSA Recv From! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}