#include "UDPSocket.h"
#include <Functions/Functions/Log/Log.h>

using namespace NETWORK::SOCKET::BASESOCKET;
using namespace NETWORK::SOCKET::UDPIP;
using namespace FUNCTIONS::LOG;

CUDPIPSocket::CUDPIPSocket() : BASESOCKET::CBaseSocket(UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP), m_ReliableThreadHandle(&CUDPIPSocket::ReliableThread, this) {
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
	}
	catch (const std::exception&) {
		
		std::abort();
	}
}

CUDPIPSocket::~CUDPIPSocket() {
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

bool CUDPIPSocket::WriteTo(const FUNCTIONS::SOCKADDR::CSocketAddress& SendAddress, const char* const SendData, const size_t& DataLength, NETWORK::UTIL::NETWORKSESSION::SERVERSESSION::DETAIL::OVERLAPPED_EX& SendOverlapped) {
	DWORD SendBytes = 0;
	WSABUF SendBuffer;
	SendBuffer.buf = const_cast<char* const>(SendData);
	SendBuffer.len = DataLength;

	if (WSASendTo(GetSocketHandle(), &SendBuffer, 1, &SendBytes, 0, &SendAddress, SendAddress.GetSize(), &SendOverlapped.m_Overlapped, nullptr) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
			CLog::WriteLog(L"WSA Send To : Failed To WSA Send To! - %d", WSAGetLastError());
			return false;
		}
	}
	return true;
}

void CUDPIPSocket::ReliableThread() {
	while (true) {
		if (!m_ReliableDataQueue.IsEmpty()) {
			if (WaitForSingleObject(m_hNewReliableDataEvent, INFINITE) != WAIT_OBJECT_0) {
				CLog::WriteLog(L"");
				continue;
			}
		}

		std::unique_ptr<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::ReliableData> QueueData;
		if (m_ReliableDataQueue.Pop(QueueData) && QueueData) {
			for (size_t i = 0; i < REPEAT_COUNT_FOR_RELIABLE_SEND; i++) {
				// Send

				if (WaitForSingleObject(m_hSendCompleteEvent, 10) == WAIT_OBJECT_0) {
					break;
				}
			}
		}
	}
}