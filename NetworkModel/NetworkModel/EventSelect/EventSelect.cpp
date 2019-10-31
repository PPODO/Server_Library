#include "EventSelect.h"
#include <Functions/Functions/Exception/Exception.h>

using namespace FUNCTIONS::LOG;

NETWORKMODEL::EVENTSELECT::CEventSelect::CEventSelect(const int PacketProcessLoopCount, const DETAIL::PACKETPROCESSORLIST& ProcessorList) : DETAIL::CNetworkModel(ProcessorList), m_PacketProcessLoopCount(PacketProcessLoopCount), m_hStopEvent(INVALID_HANDLE_VALUE), m_hTCPSelectEvent(INVALID_HANDLE_VALUE), m_hUDPSelectEvent(INVALID_HANDLE_VALUE), m_ThreadRunState(1), m_NextSendPacketNumber(0) {
}

NETWORKMODEL::EVENTSELECT::CEventSelect::~CEventSelect() {
}

bool NETWORKMODEL::EVENTSELECT::CEventSelect::Initialize(const NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE& ProtocolType, const FUNCTIONS::SOCKADDR::CSocketAddress& ServerAddress) {
	try {
		if (ProtocolType & NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP) {
			m_TCPIPSocket = std::make_unique<NETWORK::SOCKET::TCPIP::CTCPIPSocket>();
		}
		if (ProtocolType & NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP) {
			m_UDPIPSocket = std::make_unique<NETWORK::SOCKET::UDPIP::CUDPIPSocket>();
		}
	}
	catch (const std::exception& Exception) {
		CLog::WriteLog(L"%S", Exception.what());
	}

	if (!(m_hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
		CLog::WriteLog(L"");
		return false;
	}

	if (HANDLE hTCPEventSelect; m_TCPIPSocket && m_TCPIPSocket->Connect(m_ServerAddress)) {
		if (!(hTCPEventSelect = WSACreateEvent())) {
			CLog::WriteLog(L"");
			return false;
		}
		m_hTCPSelectEvent = hTCPEventSelect;
		WSAEventSelect(m_TCPIPSocket->GetSocket(), m_hTCPSelectEvent, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);

		m_EventSelectThread.emplace_back(std::thread(&NETWORKMODEL::EVENTSELECT::CEventSelect::EventSelectProcessorForTCP, this, m_hTCPSelectEvent));
	}

	if (HANDLE hUDPEventSelect; m_UDPIPSocket) {
		if (!(hUDPEventSelect = WSACreateEvent())) {
			CLog::WriteLog(L"");
			return false;
		}
		m_hUDPSelectEvent = hUDPEventSelect;
		WSAEventSelect(m_UDPIPSocket->GetSocket(), m_hUDPSelectEvent, FD_READ | FD_WRITE);

		m_EventSelectThread.emplace_back(std::thread(&NETWORKMODEL::EVENTSELECT::CEventSelect::EventSelectProcessorForUDP, this, m_hUDPSelectEvent));
	}

	return true;
}

void NETWORKMODEL::EVENTSELECT::CEventSelect::Run() {
	for (int i = 0; i < m_PacketProcessLoopCount; i++) {
		if (auto PacketData(GetPacketDataFromQueue()); PacketData) {
			if (auto Processor(GetProcessorFromList(PacketData->m_PacketStructure.m_PacketInformation.m_PacketType)); Processor) {
				Processor(PacketData);
			}
		}
	}
}

void NETWORKMODEL::EVENTSELECT::CEventSelect::Destroy() {
	InterlockedExchange16(&m_ThreadRunState, 0);

	for (auto& Iterator : m_EventSelectThread) {
		SetEvent(m_hStopEvent);
	}
	for (auto& Iterator : m_EventSelectThread) {
		if (Iterator.joinable()) {
			Iterator.join();
		}
	}
	m_EventSelectThread.clear();

	if (m_hStopEvent) {
		CloseHandle(m_hStopEvent);
	}

	if (m_hTCPSelectEvent) {
		WSACloseEvent(m_hTCPSelectEvent);
	}
	if (m_hUDPSelectEvent) {
		WSACloseEvent(m_hUDPSelectEvent);
	}

}

void NETWORKMODEL::EVENTSELECT::CEventSelect::EventSelectProcessorForTCP(const HANDLE& SelectEventHandle) {
	HANDLE hEvents[2] = { m_hStopEvent, SelectEventHandle };
	char ReceivedBuffer[NETWORK::SOCKET::BASESOCKET::MAX_RECEIVE_BUFFER_SIZE] = { "\0" };
	int16_t RemainReceivedBytes = 0;	
	int16_t LastReceivedPacketNumber = 0;

	while (m_ThreadRunState) {
		DWORD Result = WaitForMultipleObjects(2, hEvents, false, INFINITE);
		switch (WSANETWORKEVENTS NetworkEvent; Result) {
		case WAIT_OBJECT_0:
			return;
		case WAIT_OBJECT_0 + 1:
			WSAEnumNetworkEvents(m_TCPIPSocket->GetSocket(), SelectEventHandle, &NetworkEvent);

			if (NetworkEvent.lNetworkEvents & FD_CONNECT) {
				FUNCTIONS::LOG::CLog::WriteLog(L"Connect To Server!");
			}
			else if (NetworkEvent.lNetworkEvents & FD_READ) {
				uint16_t RecvBytes = 0;
				if (m_TCPIPSocket->Read(ReceivedBuffer + RemainReceivedBytes, RecvBytes)) {
					RemainReceivedBytes += RecvBytes;
					PacketForwardingLoop(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_TCP, nullptr, ReceivedBuffer, RemainReceivedBytes, LastReceivedPacketNumber);
				}
			}
			else if (NetworkEvent.lNetworkEvents & FD_WRITE) {
				// EventSelect에서의 Write는 데이터를 전송하지 않았을 때에도 발생할 수 있기에 따로 처리를 해주지 않는다.
			}
			else if (NetworkEvent.lNetworkEvents & FD_CLOSE) {
				
			}
			break;
		}
	}
}

void NETWORKMODEL::EVENTSELECT::CEventSelect::EventSelectProcessorForUDP(const HANDLE& SelectEventHandle) {
	HANDLE hEvents[2] = { m_hStopEvent, SelectEventHandle };
	char ReceivedBuffer[NETWORK::SOCKET::BASESOCKET::MAX_RECEIVE_BUFFER_SIZE] = { "\0" };
	int16_t RemainReceivedBytes = 0;

	while (m_ThreadRunState) {
		DWORD Result = WaitForMultipleObjects(2, hEvents, false, INFINITE);
		switch (WSANETWORKEVENTS NetworkEvent; Result) {
		case WAIT_OBJECT_0:
			return;
		case WAIT_OBJECT_0 + 1:
			WSAEnumNetworkEvents(m_UDPIPSocket->GetSocket(), SelectEventHandle, &NetworkEvent);

			if (NetworkEvent.lNetworkEvents & FD_READ) {
				uint16_t RecvBytes = 0;
				if (m_UDPIPSocket->ReadFrom(ReceivedBuffer + RemainReceivedBytes, RecvBytes) && NETWORK::UTIL::UDPIP::CheckAck(m_UDPIPSocket.get(), m_ServerAddress, ReceivedBuffer, RecvBytes, m_NextSendPacketNumber)) {
					RemainReceivedBytes += RecvBytes;
					PacketForwardingLoop(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_UDP, nullptr, ReceivedBuffer, RemainReceivedBytes, m_NextSendPacketNumber);
				}
			}
			else if (NetworkEvent.lNetworkEvents & FD_WRITE) {
				// EventSelect에서의 Write는 데이터를 전송하지 않았을 때에도 발생할 수 있기에 따로 처리를 해주지 않는다.
			}
			break;
		}
	}
}

bool NETWORK::UTIL::UDPIP::CheckAck(NETWORK::SOCKET::UDPIP::CUDPIPSocket* const UDPSocket, const FUNCTIONS::SOCKADDR::CSocketAddress& RemoteAddress, char* const ReceviedBuffer, uint16_t& ReceivedBytes, int16_t& UpdatedPacketNumber) {
	if (UDPSocket && ReceviedBuffer) {
		const int32_t ReadedValue = *reinterpret_cast<const int32_t*>(ReceviedBuffer);
		const int16_t AckValue = static_cast<int16_t>(ReadedValue);
		const int16_t PacketNumber = static_cast<int16_t>((ReadedValue >> 16));

		if (AckValue == 9999) {
			UpdatedPacketNumber = PacketNumber;
			return UDPSocket->SendCompletion();
		}
		else if (AckValue == 0) {
			int16_t AckNumber = 9999;
			int16_t PacketNumber = UpdatedPacketNumber + 1;
			int32_t Result = ((PacketNumber << 16) | AckNumber);
			ReceivedBytes -= sizeof(ReadedValue);
			MoveMemory(ReceviedBuffer, ReceviedBuffer + sizeof(ReadedValue), ReceivedBytes);
			return UDPSocket->WriteTo(RemoteAddress, reinterpret_cast<const char* const>(&Result), sizeof(int32_t));
		}
	}
	return false;
}